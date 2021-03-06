#include "imagewindow.h"
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QFileDialog>
#include <QShortcut>
#include <QMoveEvent>
#include <QMimeData>
#include "utils.h"
#include "logging.h"

ImageWindow::ImageWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Dialog);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    auto effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(SHANDOW_R_);
    effect->setOffset(0, 0);
    effect->setColor(QColor("#409eff"));
    setGraphicsEffect(effect);

    registerShortcuts();

    connect(&edit_menu_, &ImageEditMenu::save, this, &ImageWindow::saveAs);
    connect(&edit_menu_, &ImageEditMenu::ok, [](){});
    connect(&edit_menu_, &ImageEditMenu::fix, [](){});
    connect(&edit_menu_, &ImageEditMenu::exit, [this](){ edit_menu_.hide(); editing_ = false; });

    connect(&edit_menu_, &ImageEditMenu::undo, [](){});
    connect(&edit_menu_, &ImageEditMenu::redo, [](){});
}

void ImageWindow::fix(QPixmap image)
{
    pixmap_ = image;
    size_ = pixmap_.size();

    resize(size_ + QSize{ SHANDOW_R_ * 2, SHANDOW_R_ * 2 });

    update();
    show();
}

void ImageWindow::mousePressEvent(QMouseEvent *event)
{
    if(editing_) return;

    // thumbnail_
    if(event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonDblClick) {
        thumbnail_ = !thumbnail_;
        QRect rect({0, 0}, (thumbnail_ ? QSize{125, 125} : size_ * scale_) + QSize{SHANDOW_R_ * 2, SHANDOW_R_ * 2});
        rect.moveCenter(geometry().center());
        setGeometry(rect);

        update();
    }

    setCursor(Qt::SizeAllCursor);
    begin_ = event->globalPos();
}

void ImageWindow::mouseMoveEvent(QMouseEvent* event)
{
    if(editing_) return;

    move(event->globalPos() - begin_ + pos());
    begin_ = event->globalPos();
}

void ImageWindow::wheelEvent(QWheelEvent *event)
{
    if(editing_) return;

    auto delta = (event->delta()/12000.0);         // +/-1%

    if(ctrl_) {
        opacity_ += delta;
        if(opacity_ < 0.01) opacity_ = 0.01;
        if(opacity_ > 1.00) opacity_ = 1.00;

        setWindowOpacity(opacity_);
    }
    else if(!thumbnail_) {
        scale_ += delta;
        scale_ = scale_ < 0.01 ? 0.01 : scale_;

        QRect rect({0, 0}, (size_) * scale_ + QSize{SHANDOW_R_ * 2, SHANDOW_R_ * 2});
        rect.moveCenter(geometry().center());

        setGeometry(rect);
    }

    update();
}

void ImageWindow::paintEvent(QPaintEvent *)
{
    auto pixmap = pixmap_.scaled(size_ * scale_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if(thumbnail_) {
        auto center = pixmap.rect().center();
        pixmap = pixmap.copy(center.x() - 62, center.y() - 62, 125, 125);
    }

    painter_.begin(this);
    painter_.drawPixmap(SHANDOW_R_, SHANDOW_R_, pixmap);
    painter_.end();
}

void ImageWindow::copy()
{
    QApplication::clipboard()->setPixmap(pixmap_);
}

void ImageWindow::paste()
{
    pixmap_ = QApplication::clipboard()->pixmap();
    size_ = pixmap_.size();
    resize(size_ + QSize{ SHANDOW_R_ * 2, SHANDOW_R_ * 2});
}

void ImageWindow::open()
{
    auto filename = QFileDialog::getOpenFileName(this,
                                                 tr("Open Image"),
                                                 QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                 "Image Files(*.png *.jpg *.jpeg *.bmp)");
    if(!filename.isEmpty()) {
        pixmap_ = QPixmap(filename);
        size_ = pixmap_.size();
        resize(size_ + QSize{ SHANDOW_R_ * 2, SHANDOW_R_ * 2});
    }
}

void ImageWindow::saveAs()
{
    QString default_filepath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString default_filename = "Capturer_picture_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz") + ".png";
#ifdef _WIN32
    auto filename = QFileDialog::getSaveFileName(this,
                                                 tr("Save Image"),
                                                 default_filepath + QDir::separator() + default_filename,
                                                 "PNG(*.png);;JPEG(*.jpg *.jpeg);;BMP(*.bmp)");

    if(!filename.isEmpty()) {
        pixmap_.save(filename);
    }
#elif __linux__
    auto filename = default_filepath + QDir::separator() + default_filename;

    pixmap_.save(filename);
#endif
}

void ImageWindow::recover()
{
    if(thumbnail_) return;

    opacity_ = 1.0;
    setWindowOpacity(opacity_);

    scale_ = 1.0;
    QRect rect({0, 0}, size_ + QSize{SHANDOW_R_ * 2, SHANDOW_R_ * 2});
    rect.moveCenter(geometry().center());
    setGeometry(rect);

    update();
}

void ImageWindow::contextMenuEvent(QContextMenuEvent *)
{
    QMenu *menu = new QMenu(this);

    auto copy = new QAction(tr("Copy image"));
    menu->addAction(copy);
    connect(copy, &QAction::triggered, this, &ImageWindow::copy);

    auto paste = new QAction(tr("Paste image"));
    menu->addAction(paste);
    connect(paste, &QAction::triggered, this, &ImageWindow::paste);

    menu->addSeparator();

    auto edit = new QAction("Edit");
    menu->addAction(edit);
    connect(edit, &QAction::triggered, [this](){
        if(thumbnail_) return;

        editing_ = true;
        edit_menu_.show();
        moveMenu();
    });

    menu->addSeparator();

    auto open = new QAction(tr("Open image..."));
    menu->addAction(open);
    connect(open, &QAction::triggered, this, &ImageWindow::open);

    auto save = new QAction(tr("Save as..."));
    menu->addAction(save);
    connect(save, &QAction::triggered, this, &ImageWindow::saveAs);

    menu->addSeparator();

    auto zoom = new QAction(tr("Zoom : ") + QString::number(static_cast<int>(scale_ * 100)) + "%");
    menu->addAction(zoom);

    auto opacity = new QAction(tr("Opacity : ") + QString::number(static_cast<int>(opacity_ * 100)) + "%");
    menu->addAction(opacity);

    auto recover = new QAction(tr("Recover"));
    connect(recover, &QAction::triggered, this, &ImageWindow::recover);
    menu->addAction(recover);

    menu->addSeparator();

    auto close = new QAction(tr("Close"));
    menu->addAction(close);
    connect(close, SIGNAL(triggered(bool)), this, SLOT(close()));

    menu->exec(QCursor::pos());
}

void ImageWindow::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
    moveMenu();
}

void ImageWindow::dropEvent(QDropEvent *event)
{
    auto path = event->mimeData()->urls()[0].toLocalFile();
    LOG(INFO) << path;

    scale_ = 1.0;
    pixmap_.load(path);
    size_ = pixmap_.size();
    resize(size_ + QSize{ SHANDOW_R_ * 2, SHANDOW_R_ * 2 });
    repaint();

    event->acceptProposedAction();
}

void ImageWindow::dragEnterEvent(QDragEnterEvent *event)
{
    auto mimedata = event->mimeData();
    if(mimedata->hasUrls() && QString("jpg;png;jpeg;JPG;PNG;JPEG;bmp;BMP").contains(QFileInfo(mimedata->urls()[0].fileName()).suffix()))
        event->acceptProposedAction();
}

void ImageWindow::moveMenu()
{
    auto rect = geometry().adjusted(SHANDOW_R_, SHANDOW_R_, -SHANDOW_R_ - edit_menu_.width(), -SHANDOW_R_ + 5);
    edit_menu_.move(rect.bottomRight());
    edit_menu_.setSubMenuShowBelow();
}

void ImageWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
        close();
    }

    if(event->key() == Qt::Key_Control) {
        ctrl_ = true;
    }
}

void ImageWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        ctrl_ = false;
    }
}

void ImageWindow::registerShortcuts()
{
    connect(new QShortcut(Qt::CTRL + Qt::Key_C, this), &QShortcut::activated, this, &ImageWindow::copy);
    connect(new QShortcut(Qt::CTRL + Qt::Key_V, this), &QShortcut::activated, this, &ImageWindow::paste);

    connect(new QShortcut(Qt::CTRL + Qt::Key_S, this), &QShortcut::activated, this, &ImageWindow::saveAs);
    connect(new QShortcut(Qt::CTRL + Qt::Key_O, this), &QShortcut::activated, this, &ImageWindow::open);
}
