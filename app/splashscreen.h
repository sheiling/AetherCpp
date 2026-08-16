#pragma once

#include <QPixmap>
#include <QSplashScreen>
#include <QString>

class QPushButton;

class SplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
    explicit SplashScreen(QScreen *screen = nullptr);
    ~SplashScreen() override;

    /// 按像素原尺寸显示启动图（app.toml splash.imagePath 及回退路径）。
    void setOriginalPixmap(const QPixmap &source);

protected:
    void drawContents(QPainter *painter) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void slot_message(const QVariantList &list);
    void onAbortClicked();

private:
    void setupAbortButton();
    void positionAbortButton();

    QPushButton *abortBtn_ = nullptr;
    QString      statusText_;
};
