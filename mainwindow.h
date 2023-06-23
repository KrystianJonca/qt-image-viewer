#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QTreeView>
#include <QFileSystemModel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QWheelEvent>
#include <QDebug>
#include <QPointF>
#include <QPoint>
#include <QMouseEvent>
#include <QGraphicsPixmapItem>
#include <QMenuBar>
#include <QMenu>
#include <QComboBox>
#include <QPalette>
#include <QDialog>
#include <QAction>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void folderSelected(const QModelIndex &index);
    void imageSelected(const QModelIndex &index);
    void applyImageFilters(int index);
    void rotateImageLeft();
    void rotateImageRight();
    void fullScreen();
    void save();

private:
    Ui::MainWindow *ui;

    // Image selecting
    QTreeView *folderTree;
    QTreeView *imageTree;
    QStringList filters;
    QComboBox *filterBox;
    QFileSystemModel *folderModel;
    QFileSystemModel *imageModel;
    QDialog *imageDialog;

    // Image viewing
    QGraphicsView *imageView;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *imagePixmap;
    QGraphicsRectItem *rectItem;
    QPointF startPos;

    // Menu
    QMenu *fileMenu;

    bool eventFilter(QObject *watched, QEvent *event);
};
#endif // MAINWINDOW_H
