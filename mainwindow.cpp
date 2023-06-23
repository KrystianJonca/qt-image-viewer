#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    folderTree(new QTreeView(this)),
    imageTree(new QTreeView(this)),
    imageView(new QGraphicsView(this)),
    folderModel(new QFileSystemModel(this)),
    imageModel(new QFileSystemModel(this)),
    scene(new QGraphicsScene(this)),
    filterBox(new QComboBox(this)),
    imageDialog(new QDialog(this)),
    rectItem(nullptr),
    imagePixmap(nullptr),
    ui(new Ui::MainWindow) {
    this->installEventFilter(this);

    ui->setupUi(this);

    imageView->setScene(scene);
    imageView->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
    imageView->setMouseTracking(true);
    imageView->viewport()->installEventFilter(this);
    folderTree->setModel(folderModel);

    filters << "*.png" << "*.jpg" << "*.jpeg";
    filterBox->addItem("All");
    filterBox->addItems(filters);

    imageModel->setFilter(QDir::Files);
    imageModel->setNameFilters(filters);
    imageModel->setNameFilterDisables(false);

    folderModel->setRootPath(QDir::rootPath());
    folderModel->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
    folderTree->setRootIndex(folderModel->index(QDir::homePath()));

    connect(folderTree, &QTreeView::clicked, this, &MainWindow::folderSelected);
    connect(imageTree, &QTreeView::clicked, this, &MainWindow::imageSelected);
    connect(filterBox, &QComboBox::currentIndexChanged, this, &MainWindow::applyImageFilters);

    imageDialog->setWindowState(Qt::WindowFullScreen);

    // Image file selection + filters layout
    QVBoxLayout *imageTreeLayout = new QVBoxLayout();
    imageTreeLayout->addWidget(filterBox);
    imageTreeLayout->addWidget(imageTree);
    QWidget *imageTreeWidget = new QWidget();
    imageTreeWidget->setLayout(imageTreeLayout);

    // Main layout
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(folderTree, 1);
    layout->addWidget(imageTreeWidget, 1);
    layout->addWidget(imageView, 2);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    fileMenu = menuBar()->addMenu("File");
    fileMenu->setEnabled(false);
    QAction *rotateRightAction = fileMenu->addAction(tr("Rotate left"));
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::rotateImageRight);
    QAction *rotateLeftAction = fileMenu->addAction(tr("Rotate right"));
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateImageLeft);
    QAction *fullScreenAction = fileMenu->addAction(tr("Full screen"));
    connect(fullScreenAction, &QAction::triggered, this, &MainWindow::fullScreen);
    QAction *saveAction = fileMenu->addAction(tr("Save"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);
}

MainWindow::~MainWindow() {}

void MainWindow::folderSelected(const QModelIndex &index) {
    if(imageTree->model() == nullptr) {
        imageTree->setModel(imageModel);
    }

    QString path = folderModel->fileInfo(index).absoluteFilePath();
    imageModel->setRootPath(path);
    imageTree->setRootIndex(imageModel->index(path));
}

void MainWindow::imageSelected(const QModelIndex &index) {
    QString imagePath = imageModel->fileInfo(index).absoluteFilePath();
    QImage image(imagePath);
    if (!image.isNull()) {
        if(!fileMenu->isEnabled()) {
            fileMenu->setEnabled(true);
        }
        imageView->scene()->clear();
        imagePixmap = imageView->scene()->addPixmap(QPixmap::fromImage(image));
        imageView->fitInView(imagePixmap, Qt::KeepAspectRatio);
    }
}

void MainWindow::applyImageFilters(int index) {
    if(index == 0) {
        imageModel->setNameFilters(filters);
        return;
    }
    imageModel->setNameFilters(QStringList() << filters.at(index - 1));
}

void MainWindow::rotateImageLeft() {
    qreal rotation = imagePixmap->rotation();
    rotation -= 90;
    imagePixmap->setRotation(rotation);
    imageView->fitInView(imagePixmap, Qt::KeepAspectRatio);

}

void MainWindow::rotateImageRight() {
    qreal rotation = imagePixmap->rotation();
    rotation += 90;
    imagePixmap->setRotation(rotation);
    imageView->fitInView(imagePixmap, Qt::KeepAspectRatio);
}

void MainWindow::fullScreen() {
    QLabel *imageLabel = new QLabel(imageDialog);

    imageLabel->setPixmap(imagePixmap->pixmap());
    imageLabel->setScaledContents(true);
    imageDialog->show();
}

void MainWindow::save() {
    QImage image(imageView->viewport()->size(), QImage::Format_ARGB32);
    QPainter painter(&image);

    imageView->render(&painter);

    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "Images (*.png *.jpg)");
    if (!fileName.isEmpty()) {
        image.save(fileName);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == imageView->viewport() && imagePixmap != nullptr){
        if(event->type() == QEvent::Wheel && QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            double scaleFactor = 1.01;

            if(wheelEvent->pixelDelta().y() < 0) {
                imageView->scale(scaleFactor, scaleFactor);
            } else {
                imageView->scale(1 / scaleFactor, 1 / scaleFactor);
            }
            return true;
        }
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            startPos = imageView->mapToScene(mouseEvent->position().toPoint());
            rectItem = scene->addRect(QRectF(startPos, QSizeF()));
            rectItem->setPen(QPen(Qt::red, 2));
            return true;

        }
        if(event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            // Update statusbar position
            QPointF position = mouseEvent->globalPosition();
            QPointF positionOverPreview = imageView->mapFromGlobal(position);
            statusBar()->showMessage(QString("Mouse position: (%1, %2)").arg(positionOverPreview.x()).arg(positionOverPreview.y()));

            // Update Rect
            if (rectItem) {
                QRectF newRect(startPos, imageView->mapToScene(mouseEvent->position().toPoint()));
                rectItem->setRect(newRect.normalized());
            }
            return true;
        }
        if(event->type() == QEvent::MouseButtonRelease) {
            if (rectItem) {
                imageView->fitInView(rectItem->rect(), Qt::KeepAspectRatio);
                scene->removeItem(rectItem);
                delete rectItem;
                rectItem = nullptr;
            }
            return true;
        }

    }

    return QMainWindow::eventFilter(watched, event);
}

