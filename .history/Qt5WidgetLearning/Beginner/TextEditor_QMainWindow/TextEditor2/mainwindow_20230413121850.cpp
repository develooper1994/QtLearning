#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
    load();
}

MainWindow::~MainWindow(){
    delete ui;
}



void MainWindow::init(){
    connect(ui->actionNew, &QAction::triggered,this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered,this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered,this, &MainWindow::saveFile);
    connect(ui->actionSave_As, &QAction::triggered,this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered,this, &MainWindow::close);
    connect(ui->actionCopy, &QAction::triggered,ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionCut, &QAction::triggered,ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionPaste, &QAction::triggered,ui->plainTextEdit, &QPlainTextEdit::paste);
    connect(ui->actionSelect_All, &QAction::triggered,ui->plainTextEdit, &QPlainTextEdit::selectAll);
    connect(ui->actionSelect_None, &QAction::triggered,this, &MainWindow::selectNone);

    lblIcon = new QLabel(this);
    lblStatus = new QLabel(this);
    lblFile = new QLabel(this);

    this->setCentralWidget(ui->splitter);
    setupStatusBar();

    newFile();
    m_saved = true;

    if(ui->toolBar->isFloatable()){
        ui->toolBar->setMovable(true);
        ui->actionToolbar_floatable->setChecked(true);
        ui->actionToolbar_movable->setChecked(true);
    }

    for(int i = 0; i < 100; i++){
        QString title = "Item number " + QString::number(i);
        ui->listWidget->addItem(title);
    }
}
void MainWindow::load(){

}


void MainWindow::setupStatusBar(){
    lblIcon->setPixmap(QPixmap(":/files/images/new.png"));
    ui->statusbar->addWidget(lblIcon);

    lblStatus->setText("Not Saved:");
    ui->statusbar->addWidget(lblStatus);

    lblFile->setText("New");
    ui->statusbar->addWidget(lblFile);
}
void MainWindow::updateStatusBar(QStringView status){
    //ui->statusbar->showMessage("status");
    foreach(auto&& obj, ui->statusbar->children()){
        qDebug() << obj;
    }

    QLabel* lblIcon   = qobject_cast<QLabel*>(ui->statusbar->children().at(1));
    QLabel* lblStatus = qobject_cast<QLabel*>(ui->statusbar->children().at(2));
    QLabel* lblFile   = qobject_cast<QLabel*>(ui->statusbar->children().at(4));

    if (m_saved) {
        lblIcon->setPixmap(QPixmap(":/files/images/save.png"));
        lblStatus->setText("File Saved");
    } else {
        lblIcon->setPixmap(QPixmap(":/files/images/new.png"));
        lblStatus->setText("Not Saved");
    }

    lblFile->setText(m_filename);

}


// slots
void MainWindow::newFile(){
    ui->plainTextEdit->clear();
    m_filename.clear();
    m_saved = false;

    updateStatusBar(tr("New File"));
}
void MainWindow::openFile(){
    QString temp = QFileDialog::getOpenFileName(this,
                                                "Open File",QString(),
                                                "Text Files (*txt);;All Files (*,*)");

    if(temp.isEmpty()) return;

    m_filename = temp;
    QFile file(m_filename);
    if(!file.open(QIODevice::ReadOnly)){
        newFile();
        QMessageBox::critical(this,"Error", file.errorString());
        return;
    }

    QTextStream stream(&file);
    ui->plainTextEdit->setPlainText(stream.readAll());
    file.close();

    m_saved = true;
    updateStatusBar(m_filename);
}
void MainWindow::saveFile(){
    if(m_filename.isEmpty()){
        saveFileAs();
        return;
    }

    QFile file(m_filename);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this,"Error", file.errorString());
        return;
    }

    QTextStream stream(&file);
    stream << ui->plainTextEdit->toPlainText();
    file.close();

    m_saved = true;
    updateStatusBar(m_filename);
}
void MainWindow::saveFileAs(){
    QString temp = QFileDialog::getSaveFileName(this,"Save File",QString(),"Text Files (*txt);;All Files (*,*)");
    if(temp.isEmpty()) return;
    m_filename = temp;
    saveFile();
}
void MainWindow::selectNone(){
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    int position = cursor.position();
    cursor.clearSelection();

    cursor.setPosition(position, QTextCursor::MoveMode::KeepAnchor);
    ui->plainTextEdit->setTextCursor(cursor);
}


void MainWindow::on_actionToolbar_top_triggered(){
    addToolBar(Qt::ToolBarArea::TopToolBarArea,ui->toolBar);
}
void MainWindow::on_actionToolbar_bottom_triggered(){
    addToolBar(Qt::ToolBarArea::BottomToolBarArea,ui->toolBar);
}
void MainWindow::on_actionToolbar_left_triggered(){
    addToolBar(Qt::ToolBarArea::LeftToolBarArea,ui->toolBar);
}
void MainWindow::on_actionToolbar_right_triggered(){
    addToolBar(Qt::ToolBarArea::RightToolBarArea,ui->toolBar);
}

void MainWindow::on_actionToolbar_floatable_toggled(bool arg1){
    ui->toolBar->setFloatable(arg1);
}
void MainWindow::on_actionToolbar_movable_toggled(bool arg1){
    ui->toolBar->setMovable(arg1);
}

void MainWindow::on_plainTextEdit_textChanged(){
    m_saved=false;
    updateStatusBar(
        m_filename.isEmpty() ?
            "New File" :
            m_filename );
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item){
    ui->plainTextEdit->appendPlainText(item->text());
}

void MainWindow::on_actionAnimals_triggered(){
    QStringList options;
    options << "Cats" << "Elephant" << "Tiger" << "Antilope";

    QScopedPointer<Dialog> dialog(new Dialog());
    dialog.data()->setList(options);
    dialog->exec();  // focus on screen

    ui->plainTextEdit->insertPlainText(dialog.data()->selected());
}
void MainWindow::on_actionShapes_triggered(){
    QStringList options;
    options << "Circle" << "Rectangle" << "Heptagram" << "Curved";

    QScopedPointer<Dialog> dialog(new Dialog());
    dialog.data()->setList(options);
    dialog->exec();  // focus on screen

    ui->plainTextEdit->insertPlainText(dialog.data()->selected());
}
void MainWindow::on_actionFood_triggered(){
    QStringList options;
    options << "Avocado" << "Hamburger" << "Pizza" << "Döner";

    QScopedPointer<Dialog> dialog(new Dialog());
    dialog.data()->setList(options);
    dialog->exec();  // focus on screen

    ui->plainTextEdit->insertPlainText(dialog.data()->selected());
}

// getter-setter




