/**
 * @file inspection_history_view.cpp
 * @brief 检测结果页：单行筛选 + KPI + 满宽表格（公共控件 common/ui/component）
 */

#include "inspection_history/inspection_history_view.h"

#include "inspection_history/inspection_history_view_model.h"
#include "review/defect_review_dialog.h"
#include "vm/review/defect_review_types.h"

#include "behavior/no_wheel_input_guard.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/datetime/datetime_resolution.h"
#include "component/controls/datetime/themed_datetime_edit.h"
#include "component/controls/input/themed_line_edit.h"
#include "component/controls/label/kpi_metric_card.h"
#include "component/controls/label/themed_form_label.h"
#include "component/feedback/cursor_toast.h"
#include "component/group/themed_section_panel.h"
#include "component/table/themed_table_widget.h"
#include "component/theme/theme_tokens.h"
#include "inspection_types.h"

#include "uibase/languagemanager.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <functional>

namespace aoi {
namespace {

constexpr int kColResult = 2;
constexpr int kColTime   = 5 + inspection::kInspectionDefectTypeCount;
constexpr int kColDetail = kColTime + 1;
constexpr int kColCount  = kColDetail + 1;

/** 检测结果列语义色（与复判界面 okSwatchColor / dangerText / warningText 一致） */
constexpr int kInspectionResultColorRole = Qt::UserRole + 810;

constexpr int kFilterControlH = 36;
constexpr int kFilterRowPadV  = 6;
constexpr int kPagerControlH  = 36;
constexpr int kFilterModelW   = 132;
constexpr int kFilterSnW      = 132;
constexpr int kFilterComboW   = 108;
constexpr int kFilterDefectW  = 120;
constexpr int kFilterDateW    = 158;
constexpr int kTableRowH      = 48;

/** KPI 卡片索引：0 直通率 1 总数 2 OK 3 NG 4 RW */
ui::KpiValueRole inspectionKpiValueRole(int cardIndex)
{
    switch (cardIndex) {
    case 2:
        return ui::KpiValueRole::Ok;
    case 3:
        return ui::KpiValueRole::Ng;
    case 4:
        return ui::KpiValueRole::Warning;
    default:
        return ui::KpiValueRole::Normal;
    }
}

QString historyResultText(inspection::InspectionHistoryResult result)
{
    switch (result) {
    case inspection::InspectionHistoryResult::Ok:
        return QStringLiteral("OK");
    case inspection::InspectionHistoryResult::Ng:
        return QStringLiteral("NG");
    case inspection::InspectionHistoryResult::Rw:
        return QStringLiteral("RW");
    }
    return QStringLiteral("—");
}

QColor historyResultColor(inspection::InspectionHistoryResult result)
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    switch (result) {
    case inspection::InspectionHistoryResult::Ok:
        return review::okSwatchColor();
    case inspection::InspectionHistoryResult::Ng:
        return t.dangerText.isValid() ? t.dangerText : QColor(0xef, 0x44, 0x44);
    case inspection::InspectionHistoryResult::Rw:
        return t.warningText.isValid() ? t.warningText : QColor(0xfa, 0xcc, 0x15);
    }
    return t.textPrimary;
}

QTableWidgetItem *makeResultCell(inspection::InspectionHistoryResult result)
{
    const QColor  color = historyResultColor(result);
    const QString text  = historyResultText(result);
    auto         *item  = new QTableWidgetItem(text);
    item->setFlags(Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setData(kInspectionResultColorRole, QVariant::fromValue(color));
    item->setData(Qt::ForegroundRole, QBrush(color));
    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);
    return item;
}

QTableWidgetItem *makeCell(const QString &text, const QColor &fg = {})
{
    auto *item = new QTableWidgetItem(text);
    item->setFlags(Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    if (fg.isValid()) {
        item->setForeground(fg);
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
    }
    return item;
}

/**
 * 检测结果列自绘（复判 RejudgeResultTableDelegate / paintReviewResultCell 同款）。
 * QTableWidget QSS 的 color 会盖掉 ForegroundRole 与 palette，必须直接 drawText。
 */
void paintInspectionResultCell(QPainter *painter, const QStyleOptionViewItem &opt, const QString &text,
                               const QColor &semantic, const QWidget *widget, QStyle *style)
{
    const ui::ThemeTokens tokens = ui::ThemeTokens::resolve();

    painter->save();
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, tokens.accent);
    } else {
        // 与 QTableWidget::item QSS 一致（统一 inputBackground），勿用 AlternateBase
        painter->fillRect(opt.rect, tokens.inputBackground);
    }

    painter->setRenderHint(QPainter::TextAntialiasing, true);
    QFont f = opt.font;
    f.setBold(true);
    painter->setFont(f);
    painter->setPen(semantic);

    const QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    painter->drawText(textRect, static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

class InspectionHistoryResultTableDelegate final : public QStyledItemDelegate
{
public:
    explicit InspectionHistoryResultTableDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        if (!index.isValid() || index.column() != kColResult) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        QColor semantic = index.data(kInspectionResultColorRole).value<QColor>();
        if (!semantic.isValid()) {
            const QString text = index.data(Qt::DisplayRole).toString();
            if (text == QStringLiteral("OK"))
                semantic = review::okSwatchColor();
            else if (text == QStringLiteral("NG")) {
                const ui::ThemeTokens t = ui::ThemeTokens::resolve();
                semantic = t.dangerText.isValid() ? t.dangerText : QColor(0xef, 0x44, 0x44);
            } else if (text == QStringLiteral("RW")) {
                const ui::ThemeTokens t = ui::ThemeTokens::resolve();
                semantic = t.warningText.isValid() ? t.warningText : QColor(0xfa, 0xcc, 0x15);
            }
        }

        if (!semantic.isValid()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        const QWidget *widget = opt.widget;
        QStyle        *style  = widget ? widget->style() : QApplication::style();
        paintInspectionResultCell(painter, opt, opt.text, semantic, widget, style);
    }
};

/** 表格 cell 内嵌控件容器：不绘制底色，露出表格行背景 */
void applyTransparentWidgetSurface(QWidget *widget)
{
    if (!widget)
        return;
    widget->setAutoFillBackground(false);
    widget->setAttribute(Qt::WA_StyledBackground, false);
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    pal.setColor(QPalette::Base, Qt::transparent);
    pal.setBrush(QPalette::Window, Qt::NoBrush);
    widget->setPalette(pal);
    widget->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
}

/** 表格「查看」列：ThemedPushButton 默认尺寸 + 透明容器居中。 */
QWidget *makeDetailButtonCell(QWidget *parent, const QString &text,
                              const std::function<void()> &onClicked)
{
    auto *cell = new QWidget(parent);
    applyTransparentWidgetSurface(cell);
    cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *layout = new QHBoxLayout(cell);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(0);
    layout->addStretch(1);

    auto *btn = ui::makePushButton(text, ui::PushButtonVariant::Secondary, cell);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    QObject::connect(btn, &QPushButton::clicked, cell, [onClicked]() {
        if (onClicked)
            onClicked();
    });

    layout->addWidget(btn, 0, Qt::AlignVCenter);
    layout->addStretch(1);
    return cell;
}

void tuneLineEdit(ui::ThemedLineEdit *edit, int width)
{
    if (!edit)
        return;
    edit->setFixedHeight(kFilterControlH);
    edit->setFixedWidth(width);
    edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void tuneCombo(ui::ThemedComboBox *combo, int width)
{
    if (!combo)
        return;
    combo->setMinimumContentsLength(0);
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFixedHeight(kFilterControlH);
    combo->setMinimumHeight(kFilterControlH);
    combo->setFixedWidth(width);
    combo->setMaximumWidth(width);
    combo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

/** 分页条每页条数下拉：加宽并保证当前值可见 */
void tunePagerSizeCombo(ui::ThemedComboBox *combo)
{
    if (!combo)
        return;
    combo->setMinimumContentsLength(2);
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFixedHeight(kPagerControlH);
    combo->setMinimumHeight(kPagerControlH);
    combo->setMinimumWidth(80);
    combo->setFixedWidth(80);
    combo->setMaximumWidth(80);
    combo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

/** 筛选栏日期时间（ThemedDateTimeEdit） */
void tuneDateTimeField(ui::ThemedDateTimeEdit *edit, int width, bool endOfRange)
{
    if (!edit)
        return;
    edit->setToolbarHeight(kFilterControlH);
    edit->setResolution(ui::DateTimeResolution::Second);
    edit->setEndOfRange(endOfRange);
    edit->setFixedWidth(width);
    edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QWidget *makeFilterCell(QWidget *parent, const QString &labelKey, QWidget *editor)
{
    auto *cell = new QWidget(parent);
    cell->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    cell->setMinimumHeight(kFilterControlH + kFilterRowPadV);
    auto *hl = new QHBoxLayout(cell);
    hl->setContentsMargins(0, kFilterRowPadV / 2, 0, kFilterRowPadV / 2);
    hl->setSpacing(4);
    hl->setAlignment(Qt::AlignVCenter);
    auto *lbl = new ui::ThemedFormLabel(labelKey, cell);
    lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    hl->addWidget(lbl);
    hl->addWidget(editor);
    return cell;
}

QList<int> tableColumnStretchFactors()
{
    QList<int> factors;
    factors << 14 << 14 << 8 << 8 << 8;
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i)
        factors << 6;
    factors << 16 << 8;
    return factors;
}

} // namespace

InspectionHistoryView::InspectionHistoryView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("aoi.nav.results"));
}

void InspectionHistoryView::onLoad()
{
    setObjectName(QStringLiteral("inspectionHistoryRoot"));
    setAttribute(Qt::WA_StyledBackground, true);

    m_viewModel = new InspectionHistoryViewModel(this);
    m_statusToast = new ui::CursorToast(this);
    m_statusToast->hide();
    buildLayout();
    wireViewModel();
    syncFilterEditorsFromViewModel();
    retranslateUi();
    applyPageDecorStyles();
    applyTheme();
    ui::NoWheelInputGuard::installOn(this);
}

void InspectionHistoryView::onShow()
{
    applyFilterFromEditors();
    if (m_viewModel)
        m_viewModel->query();
}

void InspectionHistoryView::buildLayout()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(10);
    buildFilterBar(this, root);
    buildKpiRow(this, root);
    buildTableSection(this, root);
}

void InspectionHistoryView::buildFilterBar(QWidget *parent, QVBoxLayout *root)
{
    auto *section = new ui::ThemedSectionPanel(QStringLiteral("aoi.insp.filter_title"), parent);
    section->setObjectName(QStringLiteral("inspectionFilterPanel"));
    section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    auto *form = new QWidget(section->contentHost());
    form->setObjectName(QStringLiteral("inspectionFilterForm"));
    form->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    form->setMinimumHeight(kFilterControlH + kFilterRowPadV * 2);
    auto *row = new QHBoxLayout(form);
    row->setContentsMargins(0, kFilterRowPadV, 0, kFilterRowPadV);
    row->setSpacing(10);
    row->setAlignment(Qt::AlignVCenter);

    m_productEdit = ui::makeLineEdit(form);
    m_productEdit->setPlaceholderText(TR("aoi.insp.col.model"));
    tuneLineEdit(m_productEdit, kFilterModelW);

    m_snEdit = ui::makeLineEdit(form);
    m_snEdit->setPlaceholderText(TR("aoi.insp.col.sn"));
    tuneLineEdit(m_snEdit, kFilterSnW);

    m_resultCombo = ui::makeComboBox(form);
    tuneCombo(m_resultCombo, kFilterComboW);
    m_resultCombo->addItems({QStringLiteral("全部"), QStringLiteral("OK"), QStringLiteral("NG"),
                             QStringLiteral("RW")});

    m_defectCombo = ui::makeComboBox(form);
    tuneCombo(m_defectCombo, kFilterDefectW);
    m_defectCombo->addItem(QStringLiteral("全部"), QString());
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<inspection::InspectionDefectType>(i);
        m_defectCombo->addItem(TR(inspection::inspectionDefectTypeDisplayKey(type)),
                               QString::number(i));
    }

    m_timeFromEdit = new ui::ThemedDateTimeEdit(form);
    tuneDateTimeField(m_timeFromEdit, kFilterDateW, false);
    m_timeFromEdit->setDateTime(QDateTime::currentDateTime());

    m_timeToEdit = new ui::ThemedDateTimeEdit(form);
    tuneDateTimeField(m_timeToEdit, kFilterDateW, true);
    m_timeToEdit->setDateTime(QDateTime::currentDateTime());

    row->addWidget(makeFilterCell(form, QStringLiteral("aoi.insp.col.model"), m_productEdit));
    row->addWidget(makeFilterCell(form, QStringLiteral("aoi.insp.col.sn"), m_snEdit));
    row->addWidget(makeFilterCell(form, QStringLiteral("aoi.insp.col.result"), m_resultCombo));
    row->addWidget(makeFilterCell(form, QStringLiteral("aoi.review.col.type"), m_defectCombo));

    auto *timeCell = new QWidget(form);
    timeCell->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    timeCell->setMinimumHeight(kFilterControlH + kFilterRowPadV);
    auto *timeLay = new QHBoxLayout(timeCell);
    timeLay->setContentsMargins(0, kFilterRowPadV / 2, 0, kFilterRowPadV / 2);
    timeLay->setSpacing(4);
    timeLay->setAlignment(Qt::AlignVCenter);
    auto *timeLbl = new ui::ThemedFormLabel(QStringLiteral("aoi.insp.col.time"), timeCell);
    timeLbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    timeLay->addWidget(timeLbl);
    timeLay->addWidget(m_timeFromEdit);
    auto *dash = new QLabel(QStringLiteral("—"), timeCell);
    dash->setAlignment(Qt::AlignCenter);
    dash->setFixedWidth(12);
    timeLay->addWidget(dash);
    timeLay->addWidget(m_timeToEdit);
    row->addWidget(timeCell);

    row->addStretch(1);

    auto *queryBtn =
        ui::makePushButton(TR("aoi.insp.query"), ui::PushButtonVariant::Primary, form);
    auto *resetBtn =
        ui::makePushButton(TR("aoi.insp.reset"), ui::PushButtonVariant::Secondary, form);
    auto *exportBtn =
        ui::makePushButton(TR("aoi.insp.export"), ui::PushButtonVariant::Secondary, form);
    queryBtn->setFixedHeight(kFilterControlH);
    resetBtn->setFixedHeight(kFilterControlH);
    exportBtn->setFixedHeight(kFilterControlH);
    row->addWidget(queryBtn);
    row->addWidget(resetBtn);
    row->addWidget(exportBtn);

    section->contentLayout()->setContentsMargins(4, 4, 4, 10);
    section->contentLayout()->addWidget(form, 0);

    connect(queryBtn, &QPushButton::clicked, this, [this]() {
        applyFilterFromEditors();
        if (m_viewModel)
            m_viewModel->query();
        showQueryResultToast();
    });
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        if (!m_viewModel)
            return;
        m_viewModel->resetFilter();
        syncFilterEditorsFromViewModel();
        showQueryResultToast();
    });
    connect(exportBtn, &QPushButton::clicked, this, [this]() {
        if (!m_viewModel)
            return;
        applyFilterFromEditors();
        const QString path = QFileDialog::getSaveFileName(this, TR("aoi.insp.export"),
                                                          QStringLiteral("inspection_history.csv"),
                                                          QStringLiteral("CSV (*.csv)"));
        if (path.isEmpty())
            return;
        int     rowCount = 0;
        QString err;
        const bool ok = m_viewModel->exportCurrentFilter(path, &rowCount, &err);
        showExportResultToast(ok, rowCount, err);
    });

    root->addWidget(section, 0);
}

void InspectionHistoryView::buildKpiRow(QWidget *parent, QVBoxLayout *root)
{
    auto *bar = new QWidget(parent);
    bar->setObjectName(QStringLiteral("inspectionKpiBar"));
    bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *row = new QHBoxLayout(bar);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(8);

    const QStringList keys = {QStringLiteral("aoi.insp.stat.pass_rate"),
                              QStringLiteral("aoi.insp.stat.total"),
                              QStringLiteral("aoi.insp.stat.ok"),
                              QStringLiteral("aoi.insp.stat.ng"),
                              QStringLiteral("aoi.insp.stat.rw")};
    for (int i = 0; i < keys.size(); ++i) {
        auto *card = new ui::KpiMetricCard(bar);
        card->setTitle(TR(keys.at(i)));
        card->setValue(QStringLiteral("—"));
        card->setValueRole(inspectionKpiValueRole(i));
        card->setMinimumHeight(72);
        m_kpiCards.append(card);
        row->addWidget(card, 1);
    }
    root->addWidget(bar, 0);
}

void InspectionHistoryView::buildTableSection(QWidget *parent, QVBoxLayout *root)
{
    auto *section = new ui::ThemedSectionPanel(QStringLiteral("aoi.insp.data_title"), parent);
    section->setObjectName(QStringLiteral("inspectionDataPanel"));
    section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    section->contentLayout()->setContentsMargins(4, 4, 4, 4);
    section->contentLayout()->setSpacing(8);

    m_table = new ui::ThemedTableWidget(section->contentHost());
    m_table->setObjectName(QStringLiteral("inspectionHistoryTable"));
    m_table->setColumnCount(kColCount);
    m_table->setAutoRowHeightDistribution(false);
    m_table->setMinimumHeight(380);
    m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_table->verticalHeader()->setDefaultSectionSize(kTableRowH);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    if (m_table->horizontalHeader()) {
        m_table->horizontalHeader()->setMinimumHeight(40);
        m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        m_table->horizontalHeader()->setHighlightSections(false);
    }
    m_table->setItemDelegate(new InspectionHistoryResultTableDelegate(m_table));

    section->contentLayout()->addWidget(m_table, 1);
    buildPager(section);
    root->addWidget(section, 1);
}

void InspectionHistoryView::buildPager(ui::ThemedSectionPanel *section)
{
    auto *pgBar = new QWidget(section->contentHost());
    pgBar->setObjectName(QStringLiteral("inspectionPagerBar"));
    pgBar->setAttribute(Qt::WA_StyledBackground, true);
    pgBar->setMinimumHeight(44);
    auto *bar = new QHBoxLayout(pgBar);
    bar->setContentsMargins(8, 6, 8, 6);
    bar->setSpacing(10);

    m_pagerInfo = new QLabel(pgBar);
    m_pagerInfo->setObjectName(QStringLiteral("inspectionPagerTotal"));

    m_prevBtn = ui::makePushButton(QStringLiteral("上一页"), ui::PushButtonVariant::Secondary, pgBar);
    m_prevBtn->setObjectName(QStringLiteral("inspectionPagerNavBtn"));
    m_prevBtn->setFixedHeight(32);

    m_nextBtn = ui::makePushButton(QStringLiteral("下一页"), ui::PushButtonVariant::Secondary, pgBar);
    m_nextBtn->setObjectName(QStringLiteral("inspectionPagerNavBtn"));
    m_nextBtn->setFixedHeight(32);

    m_pageSizeCombo = ui::makeComboBox(pgBar);
    m_pageSizeCombo->setObjectName(QStringLiteral("inspectionPagerSizeCombo"));
    m_pageSizeCombo->addItems({QStringLiteral("10"), QStringLiteral("20"), QStringLiteral("50")});
    m_pageSizeCombo->setCurrentIndex(1);
    tunePagerSizeCombo(m_pageSizeCombo);

    bar->addStretch(1);
    bar->addWidget(m_pagerInfo);
    bar->addWidget(m_prevBtn);
    bar->addWidget(m_nextBtn);
    bar->addWidget(new QLabel(QStringLiteral("每页"), pgBar));
    bar->addWidget(m_pageSizeCombo);
    bar->addWidget(new QLabel(QStringLiteral("条"), pgBar));
    section->contentLayout()->addWidget(pgBar, 0);

    connect(m_prevBtn, &QPushButton::clicked, this, [this]() {
        if (m_viewModel)
            m_viewModel->goToPage(m_viewModel->pageIndex() - 1);
    });
    connect(m_nextBtn, &QPushButton::clicked, this, [this]() {
        if (m_viewModel)
            m_viewModel->goToPage(m_viewModel->pageIndex() + 1);
    });
    connect(m_pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (!m_viewModel || !m_pageSizeCombo)
            return;
        m_viewModel->setPageSize(m_pageSizeCombo->currentText().toInt());
    });
}

void InspectionHistoryView::wireViewModel()
{
    connect(m_viewModel, &InspectionHistoryViewModel::pageChanged, this, [this]() {
        refreshTable();
        refreshPager();
    });
    connect(m_viewModel, &InspectionHistoryViewModel::statsChanged, this, &InspectionHistoryView::refreshKpi);
}

void InspectionHistoryView::syncFilterEditorsFromViewModel()
{
    if (!m_viewModel)
        return;
    const auto &f = m_viewModel->filter();
    if (m_productEdit)
        m_productEdit->setText(f.productModel);
    if (m_snEdit)
        m_snEdit->setText(f.boardSerial);
    if (m_resultCombo) {
        const int idx = m_resultCombo->findText(f.resultFilter);
        m_resultCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    if (m_defectCombo) {
        if (f.defectTypeFilter == QStringLiteral("全部") || f.defectTypeFilter.isEmpty())
            m_defectCombo->setCurrentIndex(0);
        else {
            const int idx = m_defectCombo->findData(f.defectTypeFilter);
            m_defectCombo->setCurrentIndex(idx >= 0 ? idx : 0);
        }
    }
    if (m_timeFromEdit)
        m_timeFromEdit->setDateTime(f.timeFrom);
    if (m_timeToEdit)
        m_timeToEdit->setDateTime(f.timeTo);
}

void InspectionHistoryView::applyFilterFromEditors()
{
    if (!m_viewModel)
        return;
    if (m_productEdit)
        m_viewModel->setProductModel(m_productEdit->text());
    if (m_snEdit)
        m_viewModel->setBoardSerial(m_snEdit->text());
    if (m_resultCombo)
        m_viewModel->setResultFilter(m_resultCombo->currentText());
    if (m_defectCombo) {
        m_viewModel->setDefectTypeFilter(m_defectCombo->currentData().toString().isEmpty()
                                             ? QStringLiteral("全部")
                                             : m_defectCombo->currentData().toString());
    }
    if (m_timeFromEdit)
        m_viewModel->setTimeFrom(m_timeFromEdit->effectiveDateTime());
    if (m_timeToEdit)
        m_viewModel->setTimeTo(m_timeToEdit->effectiveDateTime());
}

void InspectionHistoryView::applyTableColumnStretch()
{
    if (!m_table)
        return;
    m_table->setHorizontalStretchFactors(tableColumnStretchFactors());
}

void InspectionHistoryView::refreshTable()
{
    if (!m_table || !m_viewModel)
        return;

    m_table->clearSpans();
    m_table->clearContents();

    const auto &page = m_viewModel->page();
    const auto &rows = page.rows;
    if (rows.isEmpty()) {
        m_table->setRowCount(1);
        m_table->setRowHeight(0, kTableRowH);
        auto *placeholder = new QTableWidgetItem(TR("aoi.insp.empty_hint"));
        placeholder->setFlags(Qt::ItemIsEnabled);
        placeholder->setTextAlignment(Qt::AlignCenter);
        const ui::ThemeTokens t = ui::ThemeTokens::resolve();
        placeholder->setForeground(t.textSecondary);
        m_table->setItem(0, 0, placeholder);
        m_table->setSpan(0, 0, 1, m_table->columnCount());
        applyTableColumnStretch();
        return;
    }

    m_table->setRowCount(rows.size());
    for (int r = 0; r < rows.size(); ++r) {
        m_table->setRowHeight(r, kTableRowH);
        const inspection::InspectionHistoryRow &d = rows.at(r);
        int c                                     = 0;
        m_table->setItem(r, c++, makeCell(d.productName));
        m_table->setItem(r, c++, makeCell(d.sn));
        m_table->setItem(r, c++, makeResultCell(d.result));
        m_table->setItem(r, c++, makeCell(QString::number(d.inspectTimes)));
        m_table->setItem(r, c++, makeCell(QString::number(d.rwCount)));
        for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i)
            m_table->setItem(r, c++, makeCell(QString::number(d.defectCounts[i])));
        m_table->setItem(r, c++, makeCell(formatDetectTime(d.detectTime)));

        const qint64 recordId = d.id;
        m_table->setCellWidget(
            r, kColDetail,
            makeDetailButtonCell(m_table, TR("aoi.insp.detail"), [this, recordId]() {
                if (!m_viewModel)
                    return;
                for (const inspection::InspectionHistoryRow &row : m_viewModel->page().rows) {
                    if (row.id == recordId) {
                        openDetailForRow(row);
                        return;
                    }
                }
            }));
    }
    applyTableColumnStretch();
}

void InspectionHistoryView::refreshKpi()
{
    if (!m_viewModel || m_kpiCards.size() < 5)
        return;
    const inspection::InspectionHistoryStats &s = m_viewModel->stats();
    m_kpiCards[0]->setValue(QStringLiteral("%1%").arg(QString::number(s.passRatePercent, 'f', 2)));
    m_kpiCards[1]->setValue(QString::number(s.totalCount));
    m_kpiCards[2]->setValue(QString::number(s.okCount));
    m_kpiCards[3]->setValue(QString::number(s.ngCount));
    m_kpiCards[4]->setValue(QString::number(s.rwCount));
    for (int i = 0; i < m_kpiCards.size(); ++i)
        m_kpiCards[i]->setValueRole(inspectionKpiValueRole(i));
}

void InspectionHistoryView::refreshPager()
{
    if (!m_viewModel || !m_pagerInfo)
        return;
    const auto &p = m_viewModel->page();
    m_pagerInfo->setText(QStringLiteral("共 %1 条 · 第 %2 / %3 页")
                             .arg(p.totalCount)
                             .arg(p.pageIndex + 1)
                             .arg(qMax(1, p.totalPages)));
    if (m_prevBtn)
        m_prevBtn->setEnabled(p.pageIndex > 0);
    if (m_nextBtn)
        m_nextBtn->setEnabled(p.pageIndex + 1 < p.totalPages);
}

void InspectionHistoryView::showQueryResultToast()
{
    if (!m_statusToast || !m_viewModel)
        return;
    const QString err = m_viewModel->lastQueryError();
    if (!err.isEmpty()) {
        m_statusToast->showMessage(TR("aoi.insp.toast.query_failed").arg(err), 3000);
        return;
    }
    m_statusToast->showMessage(
        TR("aoi.insp.toast.query_ok").arg(m_viewModel->page().totalCount), 2500);
}

void InspectionHistoryView::showExportResultToast(bool ok, int rowCount, const QString &error)
{
    if (!m_statusToast)
        return;
    if (ok)
        m_statusToast->showMessage(TR("aoi.insp.toast.export_ok").arg(rowCount), 2500);
    else
        m_statusToast->showMessage(
            TR("aoi.insp.toast.export_failed").arg(error.isEmpty() ? TR("aoi.insp.toast.unknown_error")
                                                                     : error),
            3000);
}

void InspectionHistoryView::openDetailForRow(const inspection::InspectionHistoryRow &row)
{
    if (!m_viewModel)
        return;
    if (!m_detailDialog)
        m_detailDialog = new review::DefectReviewDialog(this);
    connect(m_detailDialog, &review::DefectReviewDialog::recordUpdated, this,
            [this](qint64) {
                if (m_viewModel)
                    m_viewModel->refresh();
            },
            Qt::UniqueConnection);
    m_detailDialog->openForRecord(row);
}

QString InspectionHistoryView::formatDetectTime(const QString &iso)
{
    const QDateTime dt = QDateTime::fromString(iso, Qt::ISODate);
    if (dt.isValid())
        return dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    return iso;
}

void InspectionHistoryView::retranslateUi()
{
    setPageTitle(TR("aoi.nav.results"));
    if (!m_table)
        return;

    QStringList headers;
    headers << TR("aoi.insp.col.model") << TR("aoi.insp.col.sn") << TR("aoi.insp.col.result")
            << TR("aoi.insp.col.inspect_count") << TR("aoi.insp.col.rework_count");
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<inspection::InspectionDefectType>(i);
        headers << TR(inspection::inspectionDefectTypeDisplayKey(type));
    }
    headers << TR("aoi.insp.col.time") << TR("aoi.insp.col.detail");
    m_table->setHorizontalHeaderLabels(headers);

    const QStringList keys = {QStringLiteral("aoi.insp.stat.pass_rate"),
                              QStringLiteral("aoi.insp.stat.total"),
                              QStringLiteral("aoi.insp.stat.ok"),
                              QStringLiteral("aoi.insp.stat.ng"),
                              QStringLiteral("aoi.insp.stat.rw")};
    for (int i = 0; i < m_kpiCards.size() && i < keys.size(); ++i)
        m_kpiCards[i]->setTitle(TR(keys.at(i)));
}

void InspectionHistoryView::applyPageDecorStyles()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    if (m_pagerInfo) {
        m_pagerInfo->setStyleSheet(
            QStringLiteral("color:%1; font-size:12px; background:transparent;").arg(t.textSecondary.name()));
    }

    setStyleSheet(QStringLiteral(
        "#inspectionHistoryRoot QFrame#themedSectionPanel {"
        "  background-color:#1a2233; border:1px solid #2d3a4d; border-radius:4px; }"
        "#inspectionHistoryRoot QLabel#themedSectionPanelTitle {"
        "  color:#c8d0dc; font-size:13px; font-weight:bold; }"
        "#inspectionHistoryRoot QWidget#inspectionFilterForm {"
        "  min-height:48px; }"
        "#inspectionHistoryRoot QWidget#inspectionKpiBar QFrame#kpiMetricCard {"
        "  min-height:72px; }"
        "#inspectionHistoryRoot QTableWidget#inspectionHistoryTable {"
        "  background-color:#1e2736; alternate-background-color:#222c3c;"
        "  gridline-color:#35475c; border:1px solid #2d3a4d; font-size:12px; }"
        "#inspectionHistoryRoot QWidget#inspectionPagerBar { background:transparent; }"));
}

void InspectionHistoryView::applyTheme()
{
    PageWidget::applyTheme();
    applyPageDecorStyles();
    if (m_statusToast)
        m_statusToast->applyToastStyle();
    if (m_viewModel && m_viewModel->page().totalCount > 0) {
        refreshTable();
        refreshKpi();
        refreshPager();
    }
}

void InspectionHistoryView::resizeEvent(QResizeEvent *event)
{
    PageWidget::resizeEvent(event);
    applyTableColumnStretch();
}

} // namespace aoi
