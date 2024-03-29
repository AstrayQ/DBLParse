#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "parser.h"
#include "parsedialog.h"
#include "util.h"
#include "record.h"
#include "finder.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnWidth(0, static_cast<int>(width() * 0.5));
    m_parser = new Parser(this);
    m_finder = new Finder(this);
    Finder::init();
    connect(m_parser, &Parser::done,
            m_finder, &Finder::init);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_parser->quit();
    m_parser->wait();
}


void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::on_action_About_Dblparse_triggered()
{
    QString info = tr(R"(DBLParse<br/><br/>
DBLParse is a free and open source application that bases on dblp computer science bibliography.<br/><br/>
Please visit <a href="https://github.com/tootal/DBLParse">DBLParse</a> for more information.)");
    QMessageBox::about(this, tr("About DBLParse"), info);
}

void MainWindow::on_actionAbout_DBLP_triggered()
{
    QString info = tr(R"(DBLP<br/><br/>
The <em>dblp computer science bibliography</em> provides
open bibliographic information on major computer science journals and proceedings.
Originally created at the <a href="https://www.uni-trier.de/">University of Trier</a> in 1993,
dblp is now operated and further developed by <a href="https://www.dagstuhl.de/">Schloss Dagstuhl</a>.<br/><br/>
For more information <a href="https://dblp.uni-trier.de/faq/">check out our F.A.Q.</a>)");
    QMessageBox::about(this, tr("About DBLP"), info);
}

void MainWindow::on_actionE_xit_triggered()
{
    close();
}

void MainWindow::on_searchButton_clicked()
{
//    qDebug() << "search : " << ui->keyEdit->text();
    QString key = ui->keyEdit->text();
    if(key.isEmpty()){
        QMessageBox::information(this, tr("Information"),
                                 tr("Please enter a search key."));
        return ;
    }
    QSettings settings;
    Q_ASSERT(settings.contains("lastOpenFileName"));
    QString fileName = settings.value("lastOpenFileName").toString();
    if(ui->authorRadioButton->isChecked()){
        auto list = m_finder->indexOfAuthor(key);
        if(list.isEmpty()){
            QMessageBox::information(this, tr("Information"),
                                     tr("Author not found."));
            return ;
        }
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(list.size());
        for(int i = 0; i < list.size(); ++i){
            auto pos = list.at(i);
            Record record(Util::findRecord(fileName, pos));
//            qDebug() << record.title();
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(record.title()));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(record.mdate()));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(record.key()));
        }
        ui->tableWidget->resizeRowsToContents();
    }else if(ui->titleRadioButton->isChecked()){
        auto list = m_finder->indexOfTitle(key);
        if(list.isEmpty()){
            QMessageBox::information(this, tr("Information"),
                                     tr("Title not found."));
            return ;
        }
        ui->label->clear();
        QString text;
        for(int i = 0; i < list.size(); ++i){
            auto pos = list.at(i);
            Record record(Util::findRecord(fileName, pos));
            QString authorText;
            foreach(QString author, record.authors()){
                authorText.append(tr("Author: %1 <br/>").arg(author));
            }
            text.append(tr(R"(<b>Record details</b><br/>
Title: %1 <br/>
%2
Modify date: %3 <br/>
Key: %4 <br/><br/>
)").arg(record.title()).arg(authorText).arg(record.mdate()).arg(record.key()));
        }
        ui->label->setText(text);
    }else if(ui->coauthorRadioButton->isChecked()){
        auto list = m_finder->indexOfAuthor(key);
        if(list.isEmpty()){
            QMessageBox::information(this, tr("Information"),
                                     tr("Coauthor not found."));
            return ;
        }
        QString text;
        QStringList coauthorlist;
        for(int i = 0; i < list.size(); ++i){
            quint32 pos = list.at(i);
            Record record(Util::findRecord(fileName, pos));
            QString authorText;
            QStringList tmplist=record.coauthors();
            for(int j = 0; j< tmplist.size();++j){
               coauthorlist.append(tmplist.at(j));
            }
            record.clearCoauthors();
        }
        QSet<QString> coauthorSet = coauthorlist.toSet();
        coauthorSet.remove(key);
        foreach (const QString &value, coauthorSet)
               text.append(tr("Coauthor: %1 <br/>").arg(value));
        ui->textBrowser->setText(text);
    }
}

void MainWindow::on_action_Open_triggered()
{
    QString lastOpenFileName;
    QSettings settings;
    if(settings.contains("lastOpenFileName")){
        lastOpenFileName = settings.value("lastOpenFileName").toString();
    }else{
        // use document location as default
        lastOpenFileName = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select XML file"),
                                                    lastOpenFileName,
                                                    tr("XML file (*.xml)"));
    if(fileName.isEmpty()) return ;
    settings.setValue("lastOpenFileName", fileName);
    // question when size greater than 64MiB
    if(QFile(fileName).size() > (1 << 26)){
        QMessageBox box(this);
        box.setText(tr("Parsing the file will last for a while and will take up a lot of memory."));
        box.setInformativeText(tr("Do you want to continue?"));
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton(QMessageBox::No);
        int ret = box.exec();
        if(ret == QMessageBox::No) return ;
    }
    ParseDialog *dialog = new ParseDialog(this);
    connect(m_parser, &Parser::stateChanged,
            dialog, &ParseDialog::showStatus);
    connect(m_parser, &Parser::done,
            dialog, &ParseDialog::activeButton);
    dialog->open();
    m_parser->setFileName(fileName);
    m_parser->start();
}

void MainWindow::on_action_Status_triggered()
{
    QMessageBox msgBox(this);
    if(m_finder->parsed()){
        msgBox.setText(tr("The XML file has been parsed."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
    }else{
        msgBox.setText(tr("No XML file has been parsed."));
        msgBox.setStandardButtons(QMessageBox::Open|QMessageBox::Cancel);
        msgBox.button(QMessageBox::Open)->setText(tr("Open XML file"));
        msgBox.button(QMessageBox::Cancel)->setText(tr("Cancel"));
        msgBox.setDefaultButton(QMessageBox::Cancel);
    }
    int ret = msgBox.exec();
    if(ret == QMessageBox::Open){
        on_action_Open_triggered();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    ui->tableWidget->setColumnWidth(0, static_cast<int>(width() * 0.5));
}

void MainWindow::on_authorRadioButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->keyEdit->setFocus();
}

void MainWindow::on_titleRadioButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->keyEdit->setFocus();
}

void MainWindow::on_action_Clear_Index_triggered()
{
    m_parser->clearIndex();
    m_finder->clearIndex();
    statusBar()->showMessage(tr("Clear index file successful!"));
}

void MainWindow::on_coauthorRadioButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->keyEdit->setFocus();
}

void MainWindow::on_fuzzyRadioButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->keyEdit->setFocus();
}

void MainWindow::on_action_Settings_triggered()
{
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->open();
}
