/**
 * @file defect_review_panel.cpp
 */

#include "review/defect_review_panel.h"

#include "review/defect_review_shortcut_panel.h"
#include "vm/review/defect_review_view_model.h"

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/feedback/cursor_toast.h"
#include "component/image/image_preview.h"
#include "component/table/themed_table_widget.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "uibase/languagemanager.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QPushButton>

namespace aoi::review {
namespace {

constexpr int kRejudgeResultColorRole = Qt::UserRole + 801;
constexpr int kReviewTableRowHeight     = 28;
constexpr int kDefectPreviewHeight      = 240;

QString chipLabel(int chipType)
{
    switch (static_cast<inspection::InspectionChipType>(chipType)) {
    case inspection::InspectionChipType::R:
        return QStringLiteral("R");
    case inspection::InspectionChipType::G:
        return QStringLiteral("G");
    case inspection::InspectionChipType::B:
        return QStringLiteral("B");
    }
    return QStringLiteral("-");
}

QString maskLabel(inspection::InspectionDefectTypeMask mask)
{
    QStringList parts;
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<inspection::InspectionDefectType>(i);
        if (inspection::inspectionMaskHasDefect(mask, type))
            parts.append(TR(inspection::inspectionDefectTypeDisplayKey(type)));
    }
    return parts.join(QStringLiteral("+"));
}

QString rejudgeResultLabel(const DefectReviewLampRecord &rec)
{
    if (!rec.isRejudge)
        return TR("aoi.review.pending");
    if (rec.rejudgeDefectType == 0)
        return TR("aoi.review.result.ok");
    return maskLabel(rec.rejudgeDefectType);
}

QTableWidgetItem *ensureItem(ui::ThemedTableWidget *table, int row, int col)
{
    QTableWidgetItem *item = table->item(row, col);
    if (!item) {
        item = new QTableWidgetItem;
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(row, col, item);
    }
    return item;
}

void applyRejudgeResultColor(QTableWidgetItem *item, const DefectReviewLampRecord &rec,
                             const QColor &defaultColor, const QColor &pendingColor,
                             const QColor &okColor)
{
    if (!item)
        return;
    QColor color = defaultColor;
    if (!rec.isRejudge)
        color = pendingColor;
    else if (rec.rejudgeDefectType == 0)
        color = okColor;
    item->setData(kRejudgeResultColorRole, color);
    item->setData(Qt::ForegroundRole, QBrush(color));
}

void paintReviewResultCell(QPainter *painter, const QStyleOptionViewItem &opt, const QString &text,
                           const QColor &semantic, const QWidget *widget, QStyle *style)
{
    const ui::ThemeTokens tokens = ui::ThemeTokens::resolve();

    painter->save();
    if (opt.state & QStyle::State_Selected)
        painter->fillRect(opt.rect, tokens.accent);
    else
        painter->fillRect(opt.rect, tokens.inputBackground);

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(semantic);
    painter->setFont(opt.font);

    const QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    painter->drawText(textRect, static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

/**
 * 表格 QSS 会忽略 ForegroundRole；复判结果列自绘背景与文字（单次绘制，避免叠字发糊）。
 */
class RejudgeResultTableDelegate final : public QStyledItemDelegate
{
public:
    explicit RejudgeResultTableDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        if (!index.isValid() || index.column() != 5) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const QVariant colorVar = index.data(kRejudgeResultColorRole);
        if (!colorVar.isValid()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        const QColor semantic = colorVar.value<QColor>();
        if (!semantic.isValid()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        const QString text = opt.text;
        const QWidget  *widget = opt.widget;
        QStyle         *style  = widget ? widget->style() : QApplication::style();

        paintReviewResultCell(painter, opt, text, semantic, widget, style);
    }
};

void applyPreviewContent(ui::ImagePreview *preview, QLabel *placeholder, const QImage &image)
{
    if (!preview || !placeholder)
        return;

    if (image.isNull()) {
        preview->clearImage();
        placeholder->setVisible(true);
        return;
    }

    placeholder->setVisible(false);
    preview->setImage(image);
    preview->fitToWindow();
}

void styleImagePlaceholder(QLabel *label)
{
    if (!label)
        return;
    const ui::ThemeTokens tokens = ui::ThemeTokens::resolve();
    label->setStyleSheet(QStringLiteral("color: %1; background: transparent;")
                             .arg(tokens.textSecondary.name()));
}

} // namespace

DefectReviewPanel::DefectReviewPanel(DefectReviewViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildUi();
    m_statusToast = new ui::CursorToast(this);
    m_statusToast->hide();
    wireViewModel();
    ui::bindThemeRefresh(this, [this]() {
        syncTableFromViewModel();
        styleImagePlaceholder(m_originPlaceholder);
        styleImagePlaceholder(m_resultPlaceholder);
        if (m_statusToast)
            m_statusToast->applyToastStyle();
        retranslateUi();
    });
    retranslateUi();
    refreshFromViewModel();
}

void DefectReviewPanel::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(6);

    auto *imgRow = new QHBoxLayout();
    imgRow->setSpacing(6);

    auto *originWrap = new QVBoxLayout();
    auto *originHost = new QWidget(this);
    auto *originGrid = new QGridLayout(originHost);
    originGrid->setContentsMargins(0, 0, 0, 0);
    m_originPreview = new ui::ImagePreview(originHost);
    m_originPreview->setFixedHeight(kDefectPreviewHeight);
    m_originPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_originPlaceholder = new QLabel(TR("aoi.review.image.none"), originHost);
    m_originPlaceholder->setAlignment(Qt::AlignCenter);
    styleImagePlaceholder(m_originPlaceholder);
    originGrid->addWidget(m_originPreview, 0, 0);
    originGrid->addWidget(m_originPlaceholder, 0, 0);
    originWrap->addWidget(new QLabel(QStringLiteral("originTitle"), this));
    originWrap->addWidget(originHost);

    auto *resultWrap = new QVBoxLayout();
    auto *resultHost = new QWidget(this);
    auto *resultGrid = new QGridLayout(resultHost);
    resultGrid->setContentsMargins(0, 0, 0, 0);
    m_resultPreview = new ui::ImagePreview(resultHost);
    m_resultPreview->setFixedHeight(kDefectPreviewHeight);
    m_resultPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_resultPlaceholder = new QLabel(TR("aoi.review.image.none"), resultHost);
    m_resultPlaceholder->setAlignment(Qt::AlignCenter);
    styleImagePlaceholder(m_resultPlaceholder);
    resultGrid->addWidget(m_resultPreview, 0, 0);
    resultGrid->addWidget(m_resultPlaceholder, 0, 0);
    resultWrap->addWidget(new QLabel(QStringLiteral("resultTitle"), this));
    resultWrap->addWidget(resultHost);

    imgRow->addLayout(originWrap, 1);
    imgRow->addLayout(resultWrap, 1);
    root->addLayout(imgRow, 0);

    auto *shortcutBar = new QHBoxLayout();
    shortcutBar->setContentsMargins(0, 0, 0, 0);
    shortcutBar->setSpacing(0);
    m_shortcutPanel = new DefectReviewShortcutPanel(this);
    shortcutBar->addWidget(m_shortcutPanel, 0, Qt::AlignVCenter);
    shortcutBar->addStretch(1);
    m_confirmButton = new ui::ThemedPushButton(TR("aoi.review.confirm"),
                                               ui::PushButtonVariant::Primary, this);
    m_confirmButton->setObjectName(QStringLiteral("defectReviewConfirm"));
    m_confirmButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    shortcutBar->addWidget(m_confirmButton, 0, Qt::AlignVCenter);
    root->addLayout(shortcutBar);

    m_table = ui::makeTableWidget(0, kTableColumnCount, this);
    m_table->setObjectName(QStringLiteral("defectReviewLampTable"));
    m_table->applyCompactFormDefaults();
    m_table->setHorizontalStretchFactors({1, 2, 2, 1, 2, 2});
    m_table->setAutoRowHeightDistribution(false);
    m_table->setMinimumRowHeight(kReviewTableRowHeight);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->setItemDelegate(new RejudgeResultTableDelegate(m_table));
    m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    root->addWidget(m_table, 1);

    connect(m_table, &QTableWidget::cellClicked, this, [this](int row, int) {
        if (m_viewModel)
            m_viewModel->selectVisibleRow(row);
    });
}

void DefectReviewPanel::wireViewModel()
{
    if (!m_viewModel)
        return;

    connect(m_viewModel, &DefectReviewViewModel::sessionChanged, this, &DefectReviewPanel::refreshFromViewModel);
    connect(m_viewModel, &DefectReviewViewModel::selectionChanged, this, &DefectReviewPanel::refreshFromViewModel);

    if (m_confirmButton) {
        connect(m_confirmButton, &QPushButton::clicked, this, [this]() {
            if (!m_viewModel)
                return;
            if (m_viewModel->confirmSave()) {
                if (m_statusToast)
                    m_statusToast->showMessage(TR("aoi.review.toast.confirm_ok"), 2500);
            } else if (!m_viewModel->canConfirm() && m_statusToast) {
                m_statusToast->showMessage(TR("aoi.review.confirm.nothing"), 2500);
            }
        });
        connect(m_viewModel, &DefectReviewViewModel::dirtyChanged, this, [this](bool dirty) {
            m_confirmButton->setEnabled(dirty && m_viewModel && m_viewModel->canConfirm());
        });
        connect(m_viewModel, &DefectReviewViewModel::confirmFailed, this, [this](const QString &msg) {
            if (m_statusToast)
                m_statusToast->showMessage(TR("aoi.review.toast.confirm_failed").arg(msg), 3000);
        });
        m_confirmButton->setEnabled(false);
    }
}

void DefectReviewPanel::refreshFromViewModel()
{
    syncTableFromViewModel();
    refreshImages();
    const int row = m_viewModel ? m_viewModel->selectedVisibleIndex() : -1;
    selectTableRow(row);
}

void DefectReviewPanel::syncTableFromViewModel()
{
    if (!m_table || !m_viewModel)
        return;

    const ui::ThemeTokens                  tokens = ui::ThemeTokens::resolve();
    const QVector<LampKey>                &keys   = m_viewModel->visibleLampKeys();
    const QHash<LampKey, DefectReviewLampRecord> &records = m_viewModel->lampRecords();
    const QColor pendingColor =
        tokens.warningText.isValid() ? tokens.warningText : QColor(0xfa, 0xcc, 0x15);
    const QColor okColor = okSwatchColor();

    m_table->setRowCount(keys.size());

    for (int row = 0; row < keys.size(); ++row) {
        m_table->setRowHeight(row, kReviewTableRowHeight);
        const LampKey                &key = keys[row];
        const DefectReviewLampRecord  rec = records.value(key);

        ensureItem(m_table, row, 0)->setText(QString::number(row + 1));
        ensureItem(m_table, row, 1)->setText(QStringLiteral("(%1,%2)").arg(key.dieRow).arg(key.dieCol));
        ensureItem(m_table, row, 2)->setText(QStringLiteral("(%1,%2)").arg(rec.groupRow).arg(rec.groupCol));
        ensureItem(m_table, row, 3)->setText(chipLabel(rec.chipType));
        ensureItem(m_table, row, 4)->setText(maskLabel(rec.defectType));

        QTableWidgetItem *resultItem = ensureItem(m_table, row, 5);
        resultItem->setText(rejudgeResultLabel(rec));

        for (int col = 0; col < kTableColumnCount; ++col) {
            QTableWidgetItem *item = m_table->item(row, col);
            item->setTextAlignment(Qt::AlignCenter);
            if (col != 5)
                item->setData(Qt::ForegroundRole, QBrush(tokens.textPrimary));
        }

        applyRejudgeResultColor(resultItem, rec, tokens.textPrimary, pendingColor, okColor);
    }
}

void DefectReviewPanel::refreshImages()
{
    if (!m_viewModel) {
        applyPreviewContent(m_originPreview, m_originPlaceholder, {});
        applyPreviewContent(m_resultPreview, m_resultPlaceholder, {});
        return;
    }

    applyPreviewContent(m_originPreview, m_originPlaceholder, m_viewModel->originImageForSelection());
    applyPreviewContent(m_resultPreview, m_resultPlaceholder, m_viewModel->resultImageForSelection());
}

void DefectReviewPanel::selectTableRow(int row)
{
    if (!m_table)
        return;

    if (row < 0 || row >= m_table->rowCount()) {
        m_table->clearSelection();
        m_table->setCurrentCell(-1, -1);
        return;
    }

    m_table->setCurrentCell(row, 0);
    m_table->selectRow(row);
}

void DefectReviewPanel::retranslateUi()
{
    const auto titles = findChildren<QLabel *>();
    for (QLabel *lbl : titles) {
        if (lbl->text() == QStringLiteral("originTitle"))
            lbl->setText(TR("aoi.review.image.origin"));
        if (lbl->text() == QStringLiteral("resultTitle"))
            lbl->setText(TR("aoi.review.image.result"));
    }

    if (m_shortcutPanel)
        m_shortcutPanel->retranslateUi();

    if (m_confirmButton)
        m_confirmButton->setText(TR("aoi.review.confirm"));

    if (m_originPlaceholder)
        m_originPlaceholder->setText(TR("aoi.review.image.none"));
    if (m_resultPlaceholder)
        m_resultPlaceholder->setText(TR("aoi.review.image.none"));

    if (m_table) {
        m_table->setHorizontalHeaderLabels({
            TR("aoi.review.col.index"),
            TR("aoi.review.col.die_pos"),
            TR("aoi.review.col.group_pos"),
            TR("aoi.review.col.chip"),
            TR("aoi.review.col.type"),
            TR("aoi.review.col.result"),
        });
    }

    syncTableFromViewModel();
}

} // namespace aoi::review
