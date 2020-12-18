/*
MIT License

Copyright (c) 2020 Maifee Ul Asad

Copyright (c) 2012 EasyPaint https://github.com/Gr1N/EasyPaint


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

GitHub repo : https://github.com/maifeeulasad/Paint

A copy of the License : https://github.com/maifeeulasad/Paint/blob/main/LICENSE
*/

#include "mainwindow.h"
#include "widgets/toolbar.h"
#include "imagearea.h"
#include "datasingleton.h"
#include "dialogs/settingsdialog.h"
#include "widgets/palettebar.h"

#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QLabel>
#include <QtEvents>
#include <QPainter>
#include <QInputDialog>
#include <QUndoGroup>
#include <QtCore/QTimer>
#include <QtCore/QMap>

MainWindow::MainWindow(QStringList filePaths, QWidget *parent)
    : QMainWindow(parent), mPrevInstrumentSetted(false)
{
    mRotX = 0;
    mRotY = 0;
    mRotZ = 0;

    QSize winSize = DataSingleton::Instance()->getWindowSize();
    if (DataSingleton::Instance()->getIsRestoreWindowSize() &&  winSize.isValid()) {
        resize(winSize);
    }

    setWindowIcon(QIcon(":/media/logo/josspaint_64.png"));

    mUndoStackGroup = new QUndoGroup(this);

    initializeMainMenu();
    initializeToolBar();
    initializePaletteBar();
    initializeStatusBar();
    initializeTabWidget();

    if(filePaths.isEmpty())
    {
        initializeNewTab();
    }
    else
    {
        for(int i(0); i < filePaths.size(); i++)
        {
            initializeNewTab(true, filePaths.at(i));
        }
    }
    qRegisterMetaType<InstrumentsEnum>("InstrumentsEnum");
    DataSingleton::Instance()->setIsInitialized();
}

MainWindow::~MainWindow()
{
    
}

void MainWindow::initializeTabWidget()
{
    mTabWidget = new QTabWidget();
    mTabWidget->setUsesScrollButtons(true);
    mTabWidget->setTabsClosable(true);
    mTabWidget->setMovable(true);
    connect(mTabWidget, &QTabWidget::currentChanged, this, &MainWindow::activateTab);
    connect(mTabWidget, &QTabWidget::currentChanged, this, &MainWindow::enableActions);
    connect(mTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    setCentralWidget(mTabWidget);
}

void MainWindow::initializeNewTab(const bool &isOpen, const QString &filePath)
{
    ImageArea *imageArea;
    QString fileName(tr("Untitled Image"));
    if(isOpen && filePath.isEmpty())
    {
        imageArea = new ImageArea(isOpen, "", this);
        fileName = imageArea->getFileName();
    }
    else if(isOpen && !filePath.isEmpty())
    {
        imageArea = new ImageArea(isOpen, filePath, this);
        fileName = imageArea->getFileName();
    }
    else
    {
        imageArea = new ImageArea(false, "", this);
    }
    if (!imageArea->getFileName().isNull())
    {
        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setAttribute(Qt::WA_DeleteOnClose);
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setWidget(imageArea);

        mTabWidget->addTab(scrollArea, fileName);
        mTabWidget->setCurrentIndex(mTabWidget->count()-1);

        mUndoStackGroup->addStack(imageArea->getUndoStack());
        connect(imageArea, &ImageArea::sendPrimaryColorView, mToolbar, &ToolBar::setPrimaryColorView);
        connect(imageArea, &ImageArea::sendSecondaryColorView, mToolbar, &ToolBar::setSecondaryColorView);
        connect(imageArea, &ImageArea::sendRestorePreviousInstrument, this, &MainWindow::restorePreviousInstrument);
        connect(imageArea, &ImageArea::sendSetInstrument, this, &MainWindow::setInstrument);
        connect(imageArea, &ImageArea::sendNewImageSize, this, &MainWindow::setNewSizeToSizeLabel);
        connect(imageArea, &ImageArea::sendCursorPos, this, &MainWindow::setNewPosToPosLabel);
        connect(imageArea, &ImageArea::sendColor, this, &MainWindow::setCurrentPipetteColor);
        connect(imageArea, &ImageArea::sendEnableCopyCutActions, this, &MainWindow::enableCopyCutActions);
        connect(imageArea, &ImageArea::sendEnableSelectionInstrument, this, &MainWindow::instumentsAct);

        setWindowTitle(QString("%1 - JossPaint").arg(fileName));
    }
    else
    {
        delete imageArea;
    }
}

void MainWindow::initializeMainMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    mNewAction = new QAction(tr("&New"), this);
    mNewAction->setIcon(QIcon::fromTheme("document-new"));
    mNewAction->setIconVisibleInMenu(true);
    connect(mNewAction, &QAction::triggered, this, &MainWindow::newAct);
    fileMenu->addAction(mNewAction);

    mOpenAction = new QAction(tr("&Open"), this);
    mOpenAction->setIcon(QIcon::fromTheme("document-open"));
    mOpenAction->setIconVisibleInMenu(true);
    connect(mOpenAction, &QAction::triggered, this, &MainWindow::openAct);
    fileMenu->addAction(mOpenAction);

    mSaveAction = new QAction(tr("&Save"), this);
    mSaveAction->setIcon(QIcon::fromTheme("document-save"));
    mSaveAction->setIconVisibleInMenu(true);
    connect(mSaveAction, &QAction::triggered, this, &MainWindow::saveAct);
    fileMenu->addAction(mSaveAction);

    mSaveAsAction = new QAction(tr("Save as..."), this);
    mSaveAsAction->setIcon(QIcon::fromTheme("document-save-as"));
    mSaveAsAction->setIconVisibleInMenu(true);
    connect(mSaveAsAction, &QAction::triggered, this, &MainWindow::saveAsAct);
    fileMenu->addAction(mSaveAsAction);

    mCloseAction = new QAction(tr("&Close"), this);
    mCloseAction->setIcon(QIcon::fromTheme("window-close"));
    mCloseAction->setIconVisibleInMenu(true);
    connect(mCloseAction, &QAction::triggered, this, &MainWindow::closeTabAct);
    fileMenu->addAction(mCloseAction);

    fileMenu->addSeparator();

    mPrintAction = new QAction(tr("&Print"), this);
    mPrintAction->setIcon(QIcon::fromTheme("document-print"));
    mPrintAction->setIconVisibleInMenu(true);
    connect(mPrintAction, &QAction::triggered, this, &MainWindow::printAct);
    fileMenu->addAction(mPrintAction);

    fileMenu->addSeparator();

    mExitAction = new QAction(tr("&Exit"), this);
    mExitAction->setIcon(QIcon::fromTheme("application-exit"));
    mExitAction->setIconVisibleInMenu(true);
    connect(mExitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(mExitAction);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    mUndoAction = mUndoStackGroup->createUndoAction(this, tr("&Undo"));
    mUndoAction->setIcon(QIcon::fromTheme("edit-undo"));
    mUndoAction->setIconVisibleInMenu(true);
    mUndoAction->setEnabled(false);
    editMenu->addAction(mUndoAction);

    mRedoAction = mUndoStackGroup->createRedoAction(this, tr("&Redo"));
    mRedoAction->setIcon(QIcon::fromTheme("edit-redo"));
    mRedoAction->setIconVisibleInMenu(true);
    mRedoAction->setEnabled(false);
    editMenu->addAction(mRedoAction);

    editMenu->addSeparator();

    mCopyAction = new QAction(tr("&Copy"), this);
    mCopyAction->setIcon(QIcon::fromTheme("edit-copy"));
    mCopyAction->setIconVisibleInMenu(true);
    mCopyAction->setEnabled(false);
    connect(mCopyAction, &QAction::triggered, this, &MainWindow::copyAct);
    editMenu->addAction(mCopyAction);

    mPasteAction = new QAction(tr("&Paste"), this);
    mPasteAction->setIcon(QIcon::fromTheme("edit-paste"));
    mPasteAction->setIconVisibleInMenu(true);
    connect(mPasteAction, &QAction::triggered, this, &MainWindow::pasteAct);
    editMenu->addAction(mPasteAction);

    mCutAction = new QAction(tr("C&ut"), this);
    mCutAction->setIcon(QIcon::fromTheme("edit-cut"));
    mCutAction->setIconVisibleInMenu(true);
    mCutAction->setEnabled(false);
    connect(mCutAction, &QAction::triggered, this, &MainWindow::cutAct);
    editMenu->addAction(mCutAction);

    editMenu->addSeparator();

    QAction *settingsAction = new QAction(tr("&Settings"), this);
    settingsAction->setShortcut(QKeySequence::Preferences);
    settingsAction->setIcon(QIcon::fromTheme("document-properties"));
    settingsAction->setIconVisibleInMenu(true);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::settingsAct);
    editMenu->addAction(settingsAction);

    mInstrumentsMenu = menuBar()->addMenu(tr("&Instruments"));

    QAction *mCursorAction = new QAction(tr("Selection"), this);
    mCursorAction->setCheckable(true);
    mCursorAction->setIcon(QIcon::fromTheme("select-rectangular"));
    connect(mCursorAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mCursorAction);
    mInstrumentsActMap.insert(CURSOR, mCursorAction);

    QAction *mEraserAction = new QAction(tr("Eraser"), this);
    mEraserAction->setCheckable(true);
    mEraserAction->setIcon(QIcon::fromTheme("tool_eraser"));
    connect(mEraserAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mEraserAction);
    mInstrumentsActMap.insert(ERASER, mEraserAction);

    QAction *mColorPickerPaletteAction = new QAction(tr("Color picker palette"), this);
    mColorPickerPaletteAction->setCheckable(true);
    mColorPickerPaletteAction->setIcon(QIcon(":/media/instruments-icons/palette.png"));
    connect(mColorPickerPaletteAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mColorPickerPaletteAction);
    mInstrumentsActMap.insert(COLORPICKERPALETTE, mColorPickerPaletteAction);

    QAction *mMagnifierAction = new QAction(tr("Magnifier"), this);
    mMagnifierAction->setCheckable(true);
    mMagnifierAction->setIcon(QIcon(":/media/instruments-icons/loupe.png"));
    connect(mMagnifierAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mMagnifierAction);
    mInstrumentsActMap.insert(MAGNIFIER, mMagnifierAction);

    QAction *mPenAction = new QAction(tr("Pen"), this);
    mPenAction->setCheckable(true);
    mPenAction->setIcon(QIcon(":/media/instruments-icons/pencil.png"));
    connect(mPenAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mPenAction);
    mInstrumentsActMap.insert(PEN, mPenAction);

    QAction *mLineAction = new QAction(tr("Line"), this);
    mLineAction->setCheckable(true);
    mLineAction->setIcon(QIcon(":/media/instruments-icons/line.png"));
    connect(mLineAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mLineAction);
    mInstrumentsActMap.insert(LINE, mLineAction);

    QAction *mSprayAction = new QAction(tr("Spray"), this);
    mSprayAction->setCheckable(true);
    mSprayAction->setIcon(QIcon(":/media/instruments-icons/spray.png"));
    connect(mSprayAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mSprayAction);
    mInstrumentsActMap.insert(SPRAY, mSprayAction);

    QAction *mFillAction = new QAction(tr("Fill"), this);
    mFillAction->setCheckable(true);
    mFillAction->setIcon(QIcon(":/media/instruments-icons/fill.png"));
    connect(mFillAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mFillAction);
    mInstrumentsActMap.insert(FILL, mFillAction);

    QAction *mRectangleAction = new QAction(tr("Rectangle"), this);
    mRectangleAction->setCheckable(true);
    mRectangleAction->setIcon(QIcon(":/media/instruments-icons/rectangle.png"));
    connect(mRectangleAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mRectangleAction);
    mInstrumentsActMap.insert(RECTANGLE, mRectangleAction);

    QAction *mEllipseAction = new QAction(tr("Ellipse"), this);
    mEllipseAction->setCheckable(true);
    mEllipseAction->setIcon(QIcon(":/media/instruments-icons/ellipse.png"));
    connect(mEllipseAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mEllipseAction);
    mInstrumentsActMap.insert(ELLIPSE, mEllipseAction);

    QAction *curveLineAction = new QAction(tr("Curve"), this);
    curveLineAction->setCheckable(true);
    curveLineAction->setIcon(QIcon(":/media/instruments-icons/curve.png"));
    connect(curveLineAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(curveLineAction);
    mInstrumentsActMap.insert(CURVELINE, curveLineAction);

    QAction *mTextAction = new QAction(tr("Text"), this);
    mTextAction->setCheckable(true);
    mTextAction->setIcon(QIcon(":/media/instruments-icons/text.png"));
    connect(mTextAction, &QAction::triggered, this, &MainWindow::instumentsAct);
    mInstrumentsMenu->addAction(mTextAction);
    mInstrumentsActMap.insert(TEXT, mTextAction);

    // TODO: Add new instrument action here

    mEffectsMenu = menuBar()->addMenu(tr("E&ffects"));

    QAction *grayEfAction = new QAction(tr("Gray"), this);
    connect(grayEfAction, &QAction::triggered, this, &MainWindow::effectsAct);
    mEffectsMenu->addAction(grayEfAction);
    mEffectsActMap.insert(GRAY, grayEfAction);

    QAction *negativeEfAction = new QAction(tr("Negative"), this);
    connect(negativeEfAction, &QAction::triggered, this, &MainWindow::effectsAct);
    mEffectsMenu->addAction(negativeEfAction);
    mEffectsActMap.insert(NEGATIVE, negativeEfAction);

    QAction *binarizationEfAction = new QAction(tr("Binarization"), this);
    connect(binarizationEfAction, &QAction::triggered, this, &MainWindow::effectsAct);
    mEffectsMenu->addAction(binarizationEfAction);
    mEffectsActMap.insert(BINARIZATION, binarizationEfAction);

    QAction *gaussianBlurEfAction = new QAction(tr("Gaussian Blur"), this);
    connect(gaussianBlurEfAction, &QAction::triggered, this, &MainWindow::effectsAct);
    mEffectsMenu->addAction(gaussianBlurEfAction);
    mEffectsActMap.insert(GAUSSIANBLUR, gaussianBlurEfAction);

    QAction *gammaEfAction = new QAction(tr("Gamma"), this);
    connect(gammaEfAction, &QAction::triggered, this, &MainWindow::effectsAct);
    mEffectsMenu->addAction(gammaEfAction);
    mEffectsActMap.insert(GAMMA, gammaEfAction);

    QAction *sharpenEfAction = new QAction(tr("Sharpen"), this);
    connect(sharpenEfAction, &QAction::triggered, this, &MainWindow::effectsAct);
    mEffectsMenu->addAction(sharpenEfAction);
    mEffectsActMap.insert(SHARPEN, sharpenEfAction);

    QAction *customEfAction = new QAction(tr("Custom"), this);
    connect(customEfAction, &QAction::triggered, this, &MainWindow::effectsAct);
    mEffectsMenu->addAction(customEfAction);
    mEffectsActMap.insert(CUSTOM, customEfAction);

    mToolsMenu = menuBar()->addMenu(tr("&Tools"));

    QAction *resizeImAction = new QAction(tr("Image size..."), this);
    connect(resizeImAction, &QAction::triggered, this, &MainWindow::resizeImageAct);
    mToolsMenu->addAction(resizeImAction);

    QAction *resizeCanAction = new QAction(tr("Canvas size..."), this);
    connect(resizeCanAction, &QAction::triggered, this, &MainWindow::resizeCanvasAct);
    mToolsMenu->addAction(resizeCanAction);

    QMenu *rotateMenu = new QMenu(tr("Rotate"));

    QAction *rotateLAction = new QAction(tr("Counter-clockwise"), this);
    rotateLAction->setIcon(QIcon::fromTheme("object-rotate-left", QIcon(":/media/actions-icons/object-rotate-left.png")));
    rotateLAction->setIconVisibleInMenu(true);
    connect(rotateLAction, &QAction::triggered, this, &MainWindow::rotateLeftImageAct);
    rotateMenu->addAction(rotateLAction);

    QAction *rotateRAction = new QAction(tr("Clockwise"), this);
    rotateRAction->setIcon(QIcon::fromTheme("object-rotate-right", QIcon(":/media/actions-icons/object-rotate-right.png")));
    rotateRAction->setIconVisibleInMenu(true);
    connect(rotateRAction, &QAction::triggered, this, &MainWindow::rotateRightImageAct);
    rotateMenu->addAction(rotateRAction);

    mToolsMenu->addMenu(rotateMenu);

    QMenu *flipMenu = new QMenu(tr("Flip"));

    QAction *flipVAction = new QAction(tr("Vertically"), this);
    flipVAction->setIcon(QIcon::fromTheme("object-rotate-left", QIcon(":/media/actions-icons/object-rotate-left.png")));
    flipVAction->setIconVisibleInMenu(true);
    connect(flipVAction, &QAction::triggered, this, &MainWindow::flipVerticalAct);
    flipMenu->addAction(flipVAction);

    QAction *flipHAction = new QAction(tr("Horizontally"), this);
    flipHAction->setIcon(QIcon::fromTheme("object-rotate-right", QIcon(":/media/actions-icons/object-rotate-right.png")));
    flipHAction->setIconVisibleInMenu(true);
    connect(flipHAction, &QAction::triggered, this, &MainWindow::flipHorizontalAct);
    flipMenu->addAction(flipHAction);

    mToolsMenu->addMenu(flipMenu);


    QMenu *zoomMenu = new QMenu(tr("Zoom"));

    mZoomInAction = new QAction(tr("Zoom In"), this);
    mZoomInAction->setIcon(QIcon::fromTheme("zoom-in", QIcon(":/media/actions-icons/zoom-in.png")));
    mZoomInAction->setIconVisibleInMenu(true);
    connect(mZoomInAction, &QAction::triggered, this, &MainWindow::zoomInAct);
    zoomMenu->addAction(mZoomInAction);

    mZoomOutAction = new QAction(tr("Zoom Out"), this);
    mZoomOutAction->setIcon(QIcon::fromTheme("zoom-out", QIcon(":/media/actions-icons/zoom-out.png")));
    mZoomOutAction->setIconVisibleInMenu(true);
    connect(mZoomOutAction, &QAction::triggered, this, &MainWindow::zoomOutAct);
    zoomMenu->addAction(mZoomOutAction);

    QAction *advancedZoomAction = new QAction(tr("Advanced Zoom..."), this);
    advancedZoomAction->setIconVisibleInMenu(true);
    connect(advancedZoomAction, &QAction::triggered, this, &MainWindow::advancedZoomAct);
    zoomMenu->addAction(advancedZoomAction);

    mToolsMenu->addMenu(zoomMenu);

    QMenu *aboutMenu = menuBar()->addMenu(tr("&About"));

    QAction *aboutAction = new QAction(tr("&About JossPaint"), this);
    aboutAction->setShortcut(QKeySequence::HelpContents);
    aboutAction->setIcon(QIcon::fromTheme("help-about", QIcon(":/media/actions-icons/help-about.png")));
    aboutAction->setIconVisibleInMenu(true);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::helpAct);
    aboutMenu->addAction(aboutAction);

    QAction *aboutQtAction = new QAction(tr("About Qt"), this);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    aboutMenu->addAction(aboutQtAction);

    updateShortcuts();
}

void MainWindow::initializeStatusBar()
{
    mStatusBar = new QStatusBar();
    setStatusBar(mStatusBar);

    mSizeLabel = new QLabel();
    mPosLabel = new QLabel();
    mColorPreviewLabel = new QLabel();
    mColorRGBLabel = new QLabel();

    mStatusBar->addPermanentWidget(mSizeLabel, -1);
    mStatusBar->addPermanentWidget(mPosLabel, 1);
    mStatusBar->addPermanentWidget(mColorPreviewLabel);
    mStatusBar->addPermanentWidget(mColorRGBLabel, -1);
}

void MainWindow::initializeToolBar()
{
    mToolbar = new ToolBar(mInstrumentsActMap, this);
    addToolBar(Qt::LeftToolBarArea, mToolbar);
    connect(mToolbar, &ToolBar::sendClearStatusBarColor, this, &MainWindow::clearStatusBarColor);
    connect(mToolbar, &ToolBar::sendClearImageSelection, this, &MainWindow::clearImageSelection);
}

void MainWindow::initializePaletteBar()
{
    mPaletteBar = new PaletteBar(mToolbar);
    //addToolBar(Qt::BottomToolBarArea, mPaletteBar);
}

ImageArea* MainWindow::getCurrentImageArea()
{
    if (mTabWidget->currentWidget()) {
        QScrollArea *tempScrollArea = qobject_cast<QScrollArea*>(mTabWidget->currentWidget());
        ImageArea *tempArea = qobject_cast<ImageArea*>(tempScrollArea->widget());
        return tempArea;
    }
    return NULL;
}

ImageArea* MainWindow::getImageAreaByIndex(int index)
{
    QScrollArea *sa = static_cast<QScrollArea*>(mTabWidget->widget(index));
    ImageArea *ia = static_cast<ImageArea*>(sa->widget());
    return ia;
}

void MainWindow::activateTab(const int &index)
{
    if(index == -1)
        return;
    mTabWidget->setCurrentIndex(index);
    getCurrentImageArea()->clearSelection();
    QSize size = getCurrentImageArea()->getImage()->size();
    mSizeLabel->setText(QString("%1 x %2").arg(size.width()).arg(size.height()));

    if(!getCurrentImageArea()->getFileName().isEmpty())
    {
        setWindowTitle(QString("%1 - JossPaint").arg(getCurrentImageArea()->getFileName()));
    }
    else
    {
        setWindowTitle(QString("%1 - JossPaint").arg(tr("Untitled Image")));
    }
    mUndoStackGroup->setActiveStack(getCurrentImageArea()->getUndoStack());
}

void MainWindow::setNewSizeToSizeLabel(const QSize &size)
{
    mSizeLabel->setText(QString("%1 x %2").arg(size.width()).arg(size.height()));
}

void MainWindow::setNewPosToPosLabel(const QPoint &pos)
{
    mPosLabel->setText(QString("%1,%2").arg(pos.x()).arg(pos.y()));
}

void MainWindow::setCurrentPipetteColor(const QColor &color)
{
    mColorRGBLabel->setText(QString("RGB: %1,%2,%3").arg(color.red())
                         .arg(color.green()).arg(color.blue()));

    QPixmap statusColorPixmap = QPixmap(10, 10);
    QPainter statusColorPainter;
    statusColorPainter.begin(&statusColorPixmap);
    statusColorPainter.fillRect(0, 0, 15, 15, color);
    statusColorPainter.end();
    mColorPreviewLabel->setPixmap(statusColorPixmap);
}

void MainWindow::clearStatusBarColor()
{
    mColorPreviewLabel->clear();
    mColorRGBLabel->clear();
}

void MainWindow::newAct()
{
    initializeNewTab();
}

void MainWindow::openAct()
{
    initializeNewTab(true);
}

void MainWindow::saveAct()
{
    getCurrentImageArea()->save();
    mTabWidget->setTabText(mTabWidget->currentIndex(), getCurrentImageArea()->getFileName().isEmpty() ?
                               tr("Untitled Image") : getCurrentImageArea()->getFileName() );
}

void MainWindow::saveAsAct()
{
    getCurrentImageArea()->saveAs();
    mTabWidget->setTabText(mTabWidget->currentIndex(), getCurrentImageArea()->getFileName().isEmpty() ?
                               tr("Untitled Image") : getCurrentImageArea()->getFileName() );
}

void MainWindow::printAct()
{
    getCurrentImageArea()->print();
}

void MainWindow::settingsAct()
{
    SettingsDialog settingsDialog(this);
    if(settingsDialog.exec() == QDialog::Accepted)
    {
        settingsDialog.sendSettingsToSingleton();
        DataSingleton::Instance()->writeSettings();
        updateShortcuts();
    }
}

void MainWindow::copyAct()
{
    if (ImageArea *imageArea = getCurrentImageArea())
        imageArea->copyImage();
}

void MainWindow::pasteAct()
{
    if (ImageArea *imageArea = getCurrentImageArea())
        imageArea->pasteImage();
}

void MainWindow::cutAct()
{
    if (ImageArea *imageArea = getCurrentImageArea())
        imageArea->cutImage();
}

void MainWindow::updateShortcuts()
{
    mNewAction->setShortcut(DataSingleton::Instance()->getFileShortcutByKey("New"));
    mOpenAction->setShortcut(DataSingleton::Instance()->getFileShortcutByKey("Open"));
    mSaveAction->setShortcut(DataSingleton::Instance()->getFileShortcutByKey("Save"));
    mSaveAsAction->setShortcut(DataSingleton::Instance()->getFileShortcutByKey("SaveAs"));
    mCloseAction->setShortcut(DataSingleton::Instance()->getFileShortcutByKey("Close"));
    mPrintAction->setShortcut(DataSingleton::Instance()->getFileShortcutByKey("Print"));
    mExitAction->setShortcut(DataSingleton::Instance()->getFileShortcutByKey("Exit"));

    mUndoAction->setShortcut(DataSingleton::Instance()->getEditShortcutByKey("Undo"));
    mRedoAction->setShortcut(DataSingleton::Instance()->getEditShortcutByKey("Redo"));
    mCopyAction->setShortcut(DataSingleton::Instance()->getEditShortcutByKey("Copy"));
    mPasteAction->setShortcut(DataSingleton::Instance()->getEditShortcutByKey("Paste"));
    mCutAction->setShortcut(DataSingleton::Instance()->getEditShortcutByKey("Cut"));

    mInstrumentsActMap[CURSOR]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Cursor"));
    mInstrumentsActMap[ERASER]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Lastic"));
    mInstrumentsActMap[COLORPICKERPALETTE]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Pipette"));
    mInstrumentsActMap[MAGNIFIER]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Loupe"));
    mInstrumentsActMap[PEN]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Pen"));
    mInstrumentsActMap[LINE]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Line"));
    mInstrumentsActMap[SPRAY]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Spray"));
    mInstrumentsActMap[FILL]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Fill"));
    mInstrumentsActMap[RECTANGLE]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Rect"));
    mInstrumentsActMap[ELLIPSE]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Ellipse"));
    mInstrumentsActMap[CURVELINE]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Curve"));
    mInstrumentsActMap[TEXT]->setShortcut(DataSingleton::Instance()->getInstrumentShortcutByKey("Text"));
    // TODO: Add new instruments' shorcuts here

    mZoomInAction->setShortcut(DataSingleton::Instance()->getToolShortcutByKey("ZoomIn"));
    mZoomOutAction->setShortcut(DataSingleton::Instance()->getToolShortcutByKey("ZoomOut"));
}

void MainWindow::effectsAct()
{
    QAction *currentAction = static_cast<QAction*>(sender());
    getCurrentImageArea()->applyEffect(mEffectsActMap.key(currentAction));
}

void MainWindow::resizeImageAct()
{
    getCurrentImageArea()->resizeImage();
}

void MainWindow::resizeCanvasAct()
{
    getCurrentImageArea()->resizeCanvas();
}

void MainWindow::rotateLeftImageAct()
{
    getCurrentImageArea()->rotateImage(false);
}

void MainWindow::rotateRightImageAct()
{
    getCurrentImageArea()->rotateImage(true);
}

void MainWindow::rotateImageAct()
{
    getCurrentImageArea()->rotateImage(mRotX,mRotY,mRotZ);
}

void MainWindow::flipVerticalAct()
{
    mRotX += 180;
    getCurrentImageArea()->rotateImage(mRotX,mRotY,mRotZ);
}

void MainWindow::flipHorizontalAct()
{
    mRotY += 180;
    getCurrentImageArea()->rotateImage(mRotX,mRotY,mRotZ);
}

void MainWindow::zoomInAct()
{
    getCurrentImageArea()->zoomImage(2.0);
    getCurrentImageArea()->setZoomFactor(2.0);
}

void MainWindow::zoomOutAct()
{
    getCurrentImageArea()->zoomImage(0.5);
    getCurrentImageArea()->setZoomFactor(0.5);
}

void MainWindow::advancedZoomAct()
{
    bool ok;
    qreal factor = QInputDialog::getDouble(this, tr("Enter zoom factor"), tr("Zoom factor:"), 2.5, 0, 1000, 5, &ok);
    if (ok)
    {
        getCurrentImageArea()->zoomImage(factor);
        getCurrentImageArea()->setZoomFactor(factor);
    }
}

void MainWindow::closeTabAct()
{
    closeTab(mTabWidget->currentIndex());
}

void MainWindow::closeTab(int index)
{
    ImageArea *ia = getImageAreaByIndex(index);
    if(ia->getEdited())
    {
        int ans = QMessageBox::warning(this, tr("Closing Tab..."),
                                       tr("File has been modified.\nDo you want to save changes?"),
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
        switch(ans)
        {
        case QMessageBox::Yes:
            if (ia->save())
                break;
            return;
        case QMessageBox::Cancel:
            return;
        }
    }
    mUndoStackGroup->removeStack(ia->getUndoStack()); //for safety
    QWidget *wid = mTabWidget->widget(index);
    mTabWidget->removeTab(index);
    delete wid;
    if (mTabWidget->count() == 0)
    {
        setWindowTitle("Empty - JossPaint");
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!isSomethingModified() || closeAllTabs())
    {
        DataSingleton::Instance()->setWindowSize(size());
        DataSingleton::Instance()->writeState();
        event->accept();
    }
    else
        event->ignore();
}

bool MainWindow::isSomethingModified()
{
    for(int i = 0; i < mTabWidget->count(); ++i)
    {
        if(getImageAreaByIndex(i)->getEdited())
            return true;
    }
    return false;
}

bool MainWindow::closeAllTabs()
{

    while(mTabWidget->count() != 0)
    {
        ImageArea *ia = getImageAreaByIndex(0);
        if(ia->getEdited())
        {
            int ans = QMessageBox::warning(this, tr("Closing Tab..."),
                                           tr("File has been modified.\nDo you want to save changes?"),
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
            switch(ans)
            {
            case QMessageBox::Yes:
                if (ia->save())
                    break;
                return false;
            case QMessageBox::Cancel:
                return false;
            }
        }
        QWidget *wid = mTabWidget->widget(0);
        mTabWidget->removeTab(0);
        delete wid;
    }
    return true;
}

void MainWindow::setAllInstrumentsUnchecked(QAction *action)
{
    clearImageSelection();
    foreach (QAction *temp, mInstrumentsActMap)
    {
        if(temp != action)
            temp->setChecked(false);
    }
}

void MainWindow::setInstrumentChecked(InstrumentsEnum instrument)
{
    setAllInstrumentsUnchecked(NULL);
    if(instrument == PEN || instrument == INSTRUMENTS_COUNT)
        return;
    mInstrumentsActMap[instrument]->setChecked(true);
}

void MainWindow::instumentsAct(bool state)
{
    QAction *currentAction = static_cast<QAction*>(sender());
    if(state)
    {
        if(currentAction == mInstrumentsActMap[COLORPICKERPALETTE] && !mPrevInstrumentSetted)
        {
            DataSingleton::Instance()->setPreviousInstrument(DataSingleton::Instance()->getInstrument());
            mPrevInstrumentSetted = true;
            getCurrentImageArea()->colorpickerPaletteClicked();
        }
        setAllInstrumentsUnchecked(currentAction);
        currentAction->setChecked(true);
        DataSingleton::Instance()->setInstrument(mInstrumentsActMap.key(currentAction));
        emit sendInstrumentChecked(mInstrumentsActMap.key(currentAction));
    }
    else
    {
        setAllInstrumentsUnchecked(NULL);
        DataSingleton::Instance()->setInstrument(PEN);
        emit sendInstrumentChecked(PEN);
        if(currentAction == mInstrumentsActMap[CURSOR])
            DataSingleton::Instance()->setPreviousInstrument(mInstrumentsActMap.key(currentAction));
    }
}

void MainWindow::enableActions(int index)
{
    //if index == -1 it means, that there is no tabs
    bool isEnable = index == -1 ? false : true;

    mToolsMenu->setEnabled(isEnable);
    mEffectsMenu->setEnabled(isEnable);
    mInstrumentsMenu->setEnabled(isEnable);
    mToolbar->setEnabled(isEnable);
    mPaletteBar->setEnabled(isEnable);

    mSaveAction->setEnabled(isEnable);
    mSaveAsAction->setEnabled(isEnable);
    mCloseAction->setEnabled(isEnable);
    mPrintAction->setEnabled(isEnable);

    if(!isEnable)
    {
        setAllInstrumentsUnchecked(NULL);
        DataSingleton::Instance()->setInstrument(PEN);
        emit sendInstrumentChecked(PEN);
    }
}

void MainWindow::enableCopyCutActions(bool enable)
{
    mCopyAction->setEnabled(enable);
    mCutAction->setEnabled(enable);
}

void MainWindow::clearImageSelection()
{
    if (getCurrentImageArea())
    {
        getCurrentImageArea()->clearSelection();
        DataSingleton::Instance()->setPreviousInstrument(PEN);
    }
}

void MainWindow::restorePreviousInstrument()
{
    setInstrumentChecked(DataSingleton::Instance()->getPreviousInstrument());
    DataSingleton::Instance()->setInstrument(DataSingleton::Instance()->getPreviousInstrument());
    emit sendInstrumentChecked(DataSingleton::Instance()->getPreviousInstrument());
    mPrevInstrumentSetted = false;
}

void MainWindow::setInstrument(InstrumentsEnum instrument)
{
    setInstrumentChecked(instrument);
    DataSingleton::Instance()->setInstrument(instrument);
    emit sendInstrumentChecked(instrument);
    mPrevInstrumentSetted = false;
}

void MainWindow::helpAct()
{
    QMessageBox::about(this, tr("About JossPaint"),
                       QString("<b>JossPaint</b> %1: %2 <br> <br> %3: "
                               "<a href=\"https://github.com/maifeeulasad/Joss-Paint\">https://github.com/maifeeulasad/Joss-Paint</a>"
                               "<br> <br>Copyright (c) 2020 JossPaint team"
                               "<br> <br>%4:<ul>"
                               "<li><a href=\"mailto:grin.minsk@gmail.com\">Nikita Grishko</a> (Gr1N)</li>"
                               "<li><a href=\"mailto:faulknercs@yandex.ru\">Artem Stepanyuk</a> (faulknercs)</li>"
                               "<li><a href=\"mailto:denis.klimenko.92@gmail.com\">Denis Klimenko</a> (DenisKlimenko)</li>"
                               "<li><a href=\"mailto:bahdan.siamionau@gmail.com\">Bahdan Siamionau</a> (Bahdan)</li>"
                               "</ul>"
                               "<br> %5")
                       .arg(tr("version")).arg("0.1.0").arg(tr("Site")).arg(tr("Authors"))
                       .arg(tr("If you like <b>JossPaint</b> and you want to share your opinion, or send a bug report, or want to suggest new features, we are waiting for you on our <a href=\"https://github.com/maifeeulasad/Joss-Paint/issues?milestone=&sort=created&direction=desc&state=open\">tracker</a>.")));
}
