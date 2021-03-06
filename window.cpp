#include "window.h"
#include "ui_window.h"
#include "3rdparty/enum.h"
#include "3rdparty/tagged_bool.h"
#include <string>
#include "install.h"
#include "cmd.h"
#include "global.h"
#include <QValidator>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QtWidgets>

Window::Window(bool f, QWidget *parent) :
    QMainWindow(parent),
    fierst(!f),
    ui(new Ui::Window)
{
    if(global->set->tbios == false && OS == 0) {
        qCritical().noquote() << tr("^This PC does not supported");
    }

    //setWindowIcon(QIcon(":/yourdroid.png"));

    qDebug().noquote() << tr("Setting window");

    ui->setupUi(this);
    setLabelSetInfo();

#if WIN
    if (QtWin::isCompositionEnabled())
        QtWin::extendFrameIntoClientArea(this, 0, 0, 0, 0);
    else
        QtWin::resetExtendedFrame(this);
#endif

    connect(ui->returnInstallButton,SIGNAL(clicked()),SLOT(returnMainWindow()));
    connect(ui->settingsMini,&QPushButton::clicked,[=](){
        if(ui->windows->currentWidget() == ui->settingsPage) on_back_settings_clicked();
        else Settings_clicked();
    });
    connect(ui->buttonAboutReturn,SIGNAL(clicked()),SLOT(returnMainWindow()));
    connect(ui->buttonReturnMainDelete,SIGNAL(clicked()),SLOT(returnMainWindow()));
    connect(ui->windows, &QStackedWidget::currentChanged, [=](){
        if(ui->windows->currentWidget() != ui->settingsPage) lastPage = ui->windows->currentWidget();
    });
    connect(this, &Window::sendMesToStausbar, this, &Window::receiveMesToStatusbar);
    //    connect(ui->progressInstall, &QProgressBar::valueChanged, [&](int val) {
    //        taskBarProgress->setValue(val);
    //    });
    //connect(ui->progressDelete, &QProgressBar::valueChanged, taskBarProgress, &QWinTaskbarProgress::setValue);

    ui->progressDelete->setHidden(true);
    ui->progressInstall->setAlignment(Qt::AlignCenter);
    ui->progressInstall->setValue(0);
    ui->progressDelete->setRange(0, 7);
    ui->progressDelete->setValue(0);
    ui->editSizeDataInstall->setValidator(new QIntValidator(100, 999999));
    ui->editDirForInstall->setValidator(new QRegExpValidator(QRegExp("[^а-яА-Я^ ]{0,999}")));
    global->insSet->execBars(ui->progressInstall, ui->progressDelete, ui->statusbar);
    ui->buttonInstallInstall->setShortcut(Qt::Key_Return);

    retranslateUi(QString::fromStdString(_langHelper::to_string(global->set->getLang())));

#if WIN
    setTaskProgress();
#endif

    if(fierst) Settings_clicked();
    else returnMainWindow();

    qDebug().noquote() << "############## STARTED ##############";

}

void Window::setLabelSetInfo() {
    QString info = QString(global->set->tbios ? "Uefi " : "Bios ") + QString(global->set->arch ? "x64" : "x86");
    info = QString("<b>") + info + "<\b>";
    ui->labelSetInfo->setText(info);
}

void Window::retranslateUi(QString lang) {
    translator.load(QString("%1/translations/yourdroid_%2").arg(qApp->applicationDirPath(), lang));
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);
    ui->labelVersion->setText(QString("<b>") + tr("Version:") + QString(" ") + global->VERSION + "<\b>");
}

Window::~Window()
{
    delete ui;
}

void Window::returnMainWindow() {
    setWindowTitle(tr("YourDroid | Main menu"));
    ui->windows->setCurrentWidget(ui->mainWindowPage);
}

void Window::Settings_clicked()
{
    if(!fierst) global->set->read_set(false);
    ui->typeBios->setCurrentIndex((int)global->set->tbios);
    ui->arch->setCurrentIndex((int)global->set->arch);
#if LINUX
    ui->winVer->setEnabled(false);
#endif
    ui->checkEnableConSettings->setChecked(global->set->getConEnable());
    ui->comboLanguageSettings->setCurrentIndex((int)global->set->getLang());
    //ui->applaySettings->setEnabled(false);
    ui->windows->setCurrentWidget(ui->settingsPage);
    setWindowTitle(tr("YourDroid | Settings"));
}

void Window::on_applaySettings_clicked()
{
    if(langChanged) {
        langChanged = false;
        //qInfo().noquote() << tr("^For applying language application should restart");
        retranslateUi(QString::fromStdString(_langHelper::to_string(
                                                 (_lang)ui->comboLanguageSettings->currentIndex())));
    }
    global->set->write_set(true, ui->arch->currentIndex(),
                           ui->typeBios->currentIndex(),
                           ui->checkEnableConSettings->checkState() == Qt::Checked,
                           (_lang)ui->comboLanguageSettings->currentIndex());
    log::setEnabledCon(ui->checkEnableConSettings->checkState() == Qt::Checked);
    setLabelSetInfo();
}

void Window::on_restoreSettings_clicked()
{
    global->set->read_set(true);
    ui->typeBios->setCurrentIndex((int)global->set->tbios);
    ui->arch->setCurrentIndex((int)global->set->arch);
    ui->checkEnableConSettings->setChecked(global->set->getConEnable());
    ui->comboLanguageSettings->setCurrentIndex((int)global->set->getLang());
}

void Window::on_buttonRefreshInstall_clicked()
{
#if WIN
    ui->comboDriveSelect->clear();
    QStringList mountedDrives = global->insSet->getDrives("where (drivetype!=5 and drivetype!=6)");
    if(!mountedDrives.isEmpty())
    {
        if(mountedDrives[0] == "error")
        {
            qWarning().noquote() << "Cannot get the mounted drives";
        }
        else
        {
            qDebug().noquote() << "Excluding c:/";
            mountedDrives.removeOne("C:\\");
            qDebug().noquote() << mountedDrives;
            if(global->set->tbios)
            {
                qDebug().noquote() << "Excluding the efi partition";
                mountedDrives.removeOne(global->set->efiMountPoint.toUpper() + '\\');
                qDebug().noquote() << mountedDrives;
            }
            ui->comboDriveSelect->addItems(mountedDrives);
        }
    }
    if(global->set->ext2fsdDrvInstalled && ui->comboDriveSelect->count() > 0)
        ui->radioInstallOnPart->setEnabled(true);
    else ui->radioInstallOnPart->setEnabled(false);


    ui->comboFlashDrivesInstall->clear();
    mountedDrives = global->insSet->getDrives("where drivetype=2");
    if(!mountedDrives.isEmpty())
    {
        if(mountedDrives[0] == "error")
        {
            qWarning().noquote() << "Cannot get the mounted removable drives";
        }
        else
        {
            for(int i = 0; i < mountedDrives.count(); i++)
            {
                if(QFile::exists(mountedDrives[i] + "yourdroid_usb_boot.cfg"))
                {
                    qDebug().noquote() << QString("%1 is a boot partition, skipping it")
                                          .arg(mountedDrives[i]);
                    mountedDrives.removeAt(i);
                    i--;
                }
            }
            ui->comboFlashDrivesInstall->addItems(mountedDrives);
        }
    }
    if(ui->comboFlashDrivesInstall->count() > 0)
        ui->radioInstallFlashDriveIns->setEnabled(true);
    else ui->radioInstallFlashDriveIns->setEnabled(false);
#elif LINUX
    ui->radioInstallOnPart->setEnabled(false);
    ui->radioInstallFlashDriveIns->setEnabled(false);
#endif
}

void Window::on_installButtonMain_clicked()
{
    ui->radioInstallOnDir->setChecked(true);

    ui->radioDataToImg->setChecked(true);

    ui->radioDownload->setEnabled(false);
    ui->radioChooseFromDisk->setChecked(true);

    //ui->progressInstall->setRange(0, 100);
    ui->comboBoot->clear();
//    if(!global->set->tbios) {
//        //ui->comboBoot->addItem("Grub legasy");
//        if(global->set->winv && global->set->os) ui->comboBoot->addItem("Grub4dos");
//    }
//    //if(!global->set->winv && global->set->os) ui->comboBoot->addItem("Windows NTLDR");
//    if(global->set->tbios) {
//        //ui->comboBoot->addItem("rEFInd");
//        ui->comboBoot->addItem("Grub2");
//    }
    bool replaceWinBtldr = false;
#if WIN
    if(!global->set->tbios) ui->comboBoot->addItem("Windows bootloader");
#endif
    ui->comboBoot->addItem("Grub2");
    if(global->set->tbios)
    {
#if WIN
        replaceWinBtldr = true;
#endif
        ui->comboBoot->addItem("Grub2 for tablets");

        qDebug().noquote() << QString("^%1|yn|").arg(QObject::tr("Is this a tablet?"));
        auto choice = log::getLastPressedButton();
        if(choice == QMessageBox::Yes)
        {
            qDebug().noquote() << "Yes. Setting grub2 for tablet the default";
            ui->comboBoot->setCurrentText("Grub2 for tablets");
        }
        else
        {
            qDebug().noquote() << "No. Setting grub2 the default";
            ui->comboBoot->setCurrentText("Grub2");
        }
    }

    ui->checkReplaceWinBootloader->setEnabled(replaceWinBtldr);

    emit on_buttonRefreshInstall_clicked();

    ui->comboVersionDown->clear();
    ui->radioDownload->setEnabled(false);
    QStringList androidNameList;
    if(!androidListDownloaded)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QLabel *label = new QLabel(this);
        QProgressBar *progressBar = new QProgressBar(this);
        QDialog dialog(this);

        layout->addWidget(label);
        layout->addWidget(progressBar);
        label->setText(QObject::tr("Downloading the list of android's urls"));
        progressBar->setRange(0, 100);
        dialog.setLayout(layout);
        dialog.setWindowTitle(QObject::tr("Downloading the list of android's urls"));

        Downloader *downloader = new Downloader(this);
        QObject *context = new QObject(this);

        auto downloadEnded = [&](){
            dialog.done(0);
        };

        QObject::connect(downloader, &Downloader::success, context, [&](){
            downloadEnded();
            qDebug().noquote() << "Android list is downloaded successfully";
            androidListDownloaded = true;
        });

        QObject::connect(downloader, &Downloader::error, context, [&](QString mess){
            downloadEnded();
            qCritical().noquote() << QObject::tr("^Error downloading the android list: %1").arg(mess);
            androidListDownloaded = false;
        });

        QObject::connect(downloader, &Downloader::updateDownloadProgress, context,
                         [&](qint64 received, qint64 total){
            int progress = received / total * 100;
            progressBar->setValue(progress);
        });

        downloader->get(qApp->applicationDirPath() + "/android_list.ini",
                       QUrl("https://raw.githubusercontent.com/YourDroid/Info/master/android_list.ini"));
        dialog.exec();
        delete context;
        delete downloader;
    }
    if(QFile::exists(qApp->applicationDirPath() + "/android_list.ini"))
    {
        QSettings s(qApp->applicationDirPath() + "/android_list.ini", QSettings::IniFormat);
        QStringList groups = s.childGroups();
        qDebug().noquote() << "Android list groups: " << groups;

        for(auto x : groups)
        {
            s.beginGroup(x);
            QUrl url = s.value("url").toString();
            qDebug().noquote() << QString("The url of %1 is %2").arg(x, url.toString());
            androidsToDownload.push_front(QPair<QString, QUrl>(x, url));
            androidNameList.push_front(x);
            s.endGroup();
        }
    }

    if(!androidNameList.isEmpty())
    {
        qDebug().noquote() << "The android list is not empty";
        ui->comboVersionDown->addItems(androidNameList);
        ui->radioDownload->setEnabled(true);
    }


    setWindowTitle(tr("YourDroid | Install"));
    ui->windows->setCurrentWidget(ui->installPage);
}

void Window::on_buttonChooseImage_clicked()
{
    QString image = QFileDialog::getOpenFileName(0, tr("Choose image for install"), "", "*.iso");
    if(image.length() > 0) {
        ui->editImageFromDisk->setText(image);
        qDebug().noquote() << qApp->translate("log", "Choose image for install: ") + image;
    }
}

void Window::on_back_settings_clicked()
{
    emit on_applaySettings_clicked();
    if(fierst) {
        fierst = !fierst;
        on_applaySettings_clicked();
        returnMainWindow();
    }
    else if(lastPage == ui->settingsPage) Settings_clicked();
    else if(lastPage == ui->mainWindowPage) returnMainWindow();
    else if(lastPage == ui->aboutPage) on_buttonAboutMain_clicked();
    else if(lastPage == ui->installPage) on_installButtonMain_clicked();
    else if(lastPage == ui->deletePage) on_deleteButtonMain_clicked();
}

void Window::on_buttonChooseDirForInstall_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(0, tr("Choose directory for install"), "");
    if(dir.length() > 0) {
        ui->editDirForInstall->setText(dir);
        qDebug().noquote() << qApp->translate("log", "Choose dir for install: ") + dir;
    }
}

void Window::on_buttonInstallInstall_clicked()
{
    QString installPath;
    if(ui->radioInstallOnPart->isChecked()) installPath = ui->comboDriveSelect->currentText();
    else if(ui->radioInstallFlashDriveIns->isChecked())
        installPath = ui->comboFlashDrivesInstall->currentText();
    else installPath = ui->editDirForInstall->text();
    installPath.replace('\\', '/');
    if(installPath.back() == '/') installPath.chop(1);
    qDebug().noquote() << QString("The install path is: %1").arg(installPath);

    QString message = QObject::tr("Are you sure?");
    if(ui->radioInstallFlashDriveIns->isChecked() &&
            !global->insSet->isUsbAlreadyFormatted(installPath))
    {
        message += ' ';
        message += QObject::tr("The flash drive is going to be formatted!");
        qWarning().noquote() << QString("^%1|+-|").arg(message);
    }
    else qDebug().noquote() << QString("^%1|+-|").arg(message);
    auto choice = log::getLastPressedButton();
    if(choice == QMessageBox::Ok)
    {
        qDebug().noquote() << "Yes";
    }
    else
    {
        qDebug().noquote() << "No. Canceling";
        return;
    }

    ui->progressInstall->setStyleSheet(QProgressBar().styleSheet());
    ui->progressInstall->setValue(0);
    auto setBlocked = [=](bool _blocked)
    {
        _blocked = !_blocked;
        ui->returnInstallButton->setEnabled(_blocked);
        ui->buttonInstallInstall->setEnabled(_blocked);
        ui->buttonRefreshInstall->setEnabled(_blocked);
    };
    setBlocked(true);
    ui->statusbar->showMessage(tr("Checking"));
    qDebug().noquote() << tr("Checking data for install...");
    QString image, dir, name;
    auto end = [=](QString mess = QObject::tr("Ready")){
        ui->statusbar->showMessage(mess);
        setBlocked(false);
    };
    connect(this, &Window::ending, this, [=](QString mess) {
        end(mess);
    });

//    auto retExpr = cmd::exec(QString("%1/data/iso-editor.exe")
//                             .arg(qApp->applicationDirPath()));
//    if(retExpr.second.contains("Too few arguments")) //It output this message if it works fine
//        qDebug().noquote() << QObject::tr("iso-editor works fine");
//    else
//    {
//        qCritical().noquote() << tr("^iso-editor doesn't work:\n%1").arg(retExpr.second);
//        end();
//        return;
//    }

    if(!ui->radioDownload->isChecked()) {
        if(ui->editImageFromDisk->text().contains(' '))
        {
            qCritical().noquote() << QObject::tr("^Image path must not contain any spaces");
            end();
            return;
        }

        if(!QFile(ui->editImageFromDisk->text()).open(QIODevice::ReadWrite)) {
            qCritical().noquote() << QObject::tr("^Can't access the image");
            end();
            return;
        }
        else qDebug().noquote() << QObject::tr("Successfully opened the image");

        if((image = ui->editImageFromDisk->text()).length() == 0) {
            qCritical().noquote() << tr("^The image is not chosen");
            end();
            return;
        }
        if(!QFile::exists(image)) {
            qCritical().noquote() << tr("^Chosen image does not exist");
            end();
            return;
        }
    }
    if((dir = ui->editDirForInstall->text()).length() == 0 && ui->radioDataToFolder->isChecked()
            && !ui->radioInstallOnPart->isChecked() && !ui->radioInstallFlashDriveIns->isChecked()) {
        qCritical().noquote() << tr("^The folder is not chosen");
        end();
        return;
    }
    if(dir.contains(' '))
    {
        qCritical().noquote() << QObject::tr("^The installation path must not contain any spaces");
        end();
        return;
    }
//    if((dir = ui->editDirForInstall->text()).length() == OS * 2 + 1 ) {
//        qCritical().noquote() << tr("^Choosen folder is root");
//        end();
//        return;
//    }
    if(!ui->editDirForInstall->hasAcceptableInput()) {
        qCritical().noquote() << tr("^Invalid path");
        end();
        return;
    }
    //    if(!(new QDir())->exists(dir)) {
    //        qCritical().noquote() << tr("^Selected folder does not exist");
    //        end();
    //        return;
    //    }
    if((name = ui->editName->text()).length() == 0) {
        qCritical().noquote() << tr("^Did not fill in the name");
        end();
        return;
    }
    if((name = ui->editSizeDataInstall->text()).length() == 0 &&
            !ui->radioDataToFolder->isChecked()) {
        qCritical().noquote() << tr("^Did not fill in the size of data.img");
        end();
        return;
    }
    for(int i = 0; i < global->insSet->systemsVector().length(); i++) {
        if(ui->editName->text() == (global->insSet->systemsVector())[i].name) {
            qCritical().noquote() << QObject::tr("^The system with written name already exists");
            end();
            return;
        }
    }
//    if(!QDir(ui->editDirForInstall->text()).isEmpty()) { //if dir is not empty
//        qWarning().noquote() << tr("^Choosen folder is not empty. Some files will overwrite. Press cancel to abort|+-|");
//        if(log::getLastPressedButton() == QMessageBox::Cancel) {
//            end();
//            return;
//        }
//    }

#if LINUX
    QPair<bool, QString> result = global->insSet->mountImage(ui->editImageFromDisk->text());
    if(!result.first) {
        qCritical().noquote() << QObject::tr("^Could not mount image: %1").arg(result.second);
        global->insSet->unmountImage();
        end();
        return;
    }
#endif

    if(!ui->radioDownload->isChecked())
    {
        int ret = 0;
        if(!(ret = global->insSet->isInvalidImage(
#if WIN
                 ui->editImageFromDisk->text()
#endif
                 ))) {
            if(ret != 2) qCritical().noquote() << QObject::tr("^Image has not needed files");
            end();
            return;
        }
    }

    qDebug().noquote() << QObject::tr("Data of install is valid");
    ui->statusbar->showMessage(QObject::tr("Data of install is valid"));

#if WIN
    taskBarProgress->resume();
#endif

#if WIN
    if(global->set->getBios()) {
        ui->statusbar->showMessage(QObject::tr("Mounting efi partition"));
        if(global->set->isEfiPartMounted())
        {
            qDebug().noquote() << QObject::tr("The efi partition has already been mounted");
        }
        else if(!global->set->mountEfiPart().first && !ui->radioInstallFlashDriveIns->isChecked())
        {
            qCritical().noquote()
                    << QObject::tr("^Could not mount efi partition. Aborting. \nTry rebooting your computer");
            end();
            return;
        }
    }
#endif

    //ui->progressInstall->setRange(0, (ui->radioChooseFromDisk->isChecked() && !ui->radioDownload->isChecked()) ? 125 : 150);
    QString boot = ui->comboBoot->currentText();
    if(boot == "Windows bootloader") boot = "win_bootloader";
    else if(boot == "Grub2 for tablets") boot = "grub2_tablet";
    else boot = boot.toLower();
    if(ui->radioInstallFlashDriveIns->isChecked()) boot = "grub2_flash";
    qDebug().noquote() << "The bootloader is " << boot;
    _bootloader bootloader = _bootloaderHelper::from_string(boot.toStdString());

    _typePlace typePlace;
    if(ui->radioInstallOnDir->isChecked()) typePlace = _typePlace::dir;
    else if (ui->radioInstallOnPart->isChecked()) typePlace = _typePlace::partition;
    else if (ui->radioInstallFlashDriveIns->isChecked()) typePlace = _typePlace::flash_drive;
    //_typePlace typePlace = ui->radioInstallOnDir->isChecked() ? _typePlace::dir : _typePlace::partition;
#define CHECK_ABORT() if(abort) return;

    //    QWinTaskbarButton *taskBarButton = new QWinTaskbarButton(this);
    //    taskBarButton->setWindow(windowHandle());

    //    QWinTaskbarProgress *taskBarProgress = taskBarButton->progress();
    //    taskBarProgress->setVisible(true);

    //installPath is defined at the begining of this function

    global->insSet->addSystem(bootloader, typePlace, installPath, ui->editImageFromDisk->text(), ui->editName->text(), false);
    QFutureWatcher<void> *resMonitor = new QFutureWatcher<void>;
    connect(resMonitor, &QFutureWatcher<void>::finished, this, [&](){
        qApp->alert(this, 2000);
        if(aborted) {
            emit ending(QObject::tr("Aborted"));
            ui->progressInstall->setStyleSheet("QProgressBar::chunk {background-color: red;}");
#if WIN
            taskBarProgress->stop();
#endif
//            disconnect(global->insSet, &install::abort, this, nullptr);
//            androidDelete();
            return;
        }
        emit ending();
        global->insSet->sysSetSuccess();
//        global->insSet->write();
        ui->progressInstall->setValue(ui->progressInstall->maximum());
#if WIN
        taskBarProgress->hide();
        taskBarProgress->pause();
#endif
        on_buttonRefreshInstall_clicked();
        qDebug().noquote() << QObject::tr("^Success");
    });

    progressSteps = 3;
    currentSteps = 1;
    if(ui->radioInstallOnPart->isChecked() || ui->radioInstallFlashDriveIns->isChecked())
        progressSteps++;
    if(ui->radioDownload->isChecked()) progressSteps++;
    int step = 100 / progressSteps;
    qDebug().noquote() << "The number of progress steps is " << progressSteps;
    qDebug().noquote() << "Progress step is " << step;

    ui->progressInstall->setRange(0, 105);
    ui->progressInstall->setValue(5);
#if WIN
    taskBarProgress->setRange(0, 105);
    taskBarProgress->setValue(5);
#endif

    connect(this, &Window::progressAddStep, this, [step, this](){
        qDebug().noquote() << "Current steps: " << currentSteps;
        ui->progressInstall->setValue(5 + step * currentSteps);
#if WIN
        taskBarProgress->setValue(5 + step * currentSteps);
#endif
        currentSteps++;
    });
    connect(global->insSet, &install::downloadProgress, this, [step, this](qint64 received, qint64 total) {
        float progress = (received * 100) / total;
        qDebug().noquote() << "The download progress is " << progress;
        int totalProgress = 5 + step * (currentSteps - 1) + progress * step / 100;
        qDebug().noquote() << "Total progress is " << totalProgress;
        ui->progressInstall->setValue(totalProgress);
#if WIN
        taskBarProgress->setValue(totalProgress);
#endif
    }, Qt::QueuedConnection);

    auto logMain = [=](QtMsgType type, QString mess){
        QDebug(type).noquote() << mess;
    };
    connect(global->insSet, &install::logWindow, this, [=](QtMsgType type, QString mess) {
        emit logFromMainThread(type, mess);
    });
    connect(this, &Window::logFromMainThread, this, logMain);
    connect(this, &Window::setAborted, this, [=](bool a) {
        aborted = a;
    });


    if(aborted) {
        aborted = false;
#if WIN
        setTaskProgress();
#endif
    }
#if WIN
    taskBarProgress->setVisible(true);
    taskBarProgress->show();
#endif
    //taskBarProgress->setValue(step);
    auto res = QtConcurrent::run([=](){ // auto - QFuture
        bool abort = false;
        connect(global->insSet, &install::abort, [&](QString mes){
            emit setAborted(true);
            global->insSet->delBackSystem();
            abort = true;
            emit logFromMainThread(QtCriticalMsg,
                                   tr("^Fatal error while installing: %1")
                                   .arg(mes.isEmpty() ? "Message about error is empty" : mes));
#if LINUX
            global->insSet->unmountImage();
#endif
        });
        qDebug().noquote() << tr("Start install");

        if(ui->radioDownload->isChecked())
        {
            qDebug().noquote() << tr("Downloading the image...");
            emit sendMesToStausbar(tr("Downloading the image..."));
            QString androidName = ui->comboVersionDown->currentText();
            qDebug().noquote() << "Chosen image is " << androidName;
            QUrl url;
            for(auto x : androidsToDownload)
            {
                if(x.first == androidName)
                {
                    url = x.second;
                    qDebug().noquote() << QString("%1 matches the chosen image. The url is %2")
                                          .arg(x.first, x.second.toString());
                    break;
                }
                else qDebug().noquote() << QString("%1 doesn't match the chosen image")
                                           .arg(x.first);
            }
            if(url.isEmpty())
            {
                emit global->insSet->abort(tr("Can't find the url of the chosen image"));
            }
            else global->insSet->downloadImage(url);

            int ret = 0;
            if(!(ret = global->insSet->isInvalidImage(
#if WIN
                     qApp->applicationDirPath() + "/android.iso"
#endif
                     ))) {
                if(ret != 2) global->insSet->abort(tr("The downloaded image doesn't contain "
                                                      "all of the required files"));
            }

            CHECK_ABORT();
            emit progressAddStep();
        }

        if(ui->radioInstallFlashDriveIns->isChecked())
        {
            qDebug().noquote() << tr("Formating flash drive and installing the bootloader (may take a while)...");
            emit sendMesToStausbar(tr("Formating flash drive and installing the bootloader (may take a while)..."));
            global->insSet->formatFlashDrive();
            CHECK_ABORT();
            emit progressAddStep();
        }
        else if(ui->radioInstallOnPart->isChecked())
        {
            qDebug().noquote() << tr("Formating the selected drive...");
            emit sendMesToStausbar(tr("Formating the selected drive..."));
            global->insSet->formatPartExt4();
            CHECK_ABORT();
            emit progressAddStep();
        }

        qDebug().noquote() << tr("Unpacking iso...");
        emit sendMesToStausbar(tr("Unpacking iso..."));
        global->insSet->unpackSystem(static_cast<sysImgExtractType>
                                     (ui->comboSysMountAs->currentIndex()));
        CHECK_ABORT();
        emit progressAddStep();

        qDebug().noquote() << tr("Creating data.img...");
        emit sendMesToStausbar(tr("Creating data.img..."));
        global->insSet->createDataImg(ui->editSizeDataInstall->text().toInt(),
                                      ui->radioDataToFolder->isChecked());
        CHECK_ABORT();
        emit progressAddStep();

        qDebug().noquote() << tr("Installing and configuring bootloader...");
        emit sendMesToStausbar(tr("Installing and configuring bootloader..."));
        global->insSet->registerBootloader(ui->checkReplaceWinBootloader->isChecked());
        CHECK_ABORT();
        emit progressAddStep();

#if LINUX
        emit sendMesToStausbar(tr("Unmounting image..."));
        emit progressAddStep();
        global->insSet->unmountImage();
#endif
        global->insSet->systemEnd();
        global->insSet->oldSysEdit() = true;
        if(typePlace == _typePlace::flash_drive)
            global->insSet->saveFlashData();
        else
            global->insSet->write();
        CHECK_ABORT();
    });
    resMonitor->setFuture(res);
    return;
#undef CHECK_ABORT
}

void Window::on_buttonAboutMain_clicked()
{
    setWindowTitle(tr("YourDroid | About"));
    ui->windows->setCurrentWidget(ui->aboutPage);
}

void Window::on_comboBoot_currentIndexChanged(const QString &arg1)
{
    qDebug().noquote() << qApp->translate("log", "Choose ") + arg1;
    //    if(arg1 == "Grub2") {
    //        ui->labelAboutBootloader->setText("Рекомендуется для компьютеров.");
    //        ui->labelAboutBootloader_2->setText("Текстовый.");
    //    }
    //    else if(arg1 == "Gummiboot") {
    //        ui->labelAboutBootloader->setText("Рекомендуется для планшетов.");
    //        ui->labelAboutBootloader_2->setText("Управление качалкой громкости.");
    //    }
    //    else if(arg1 == "rEFInd") ui->labelAboutBootloader->setText("Поддержка сенс. экрана, работает не везде");
    //    else if(arg1 == "Windows BOOTMGR") ui->labelAboutBootloader->setText("Рекомендуется для планшетов, c win 8+. В win 8+ сенсорный");
    //    else if(arg1 == "Windows NTDLR") ui->labelAboutBootloader->setText("Не рекомендуется. Текстовый");
    //    else if(arg1 == "Grub legasy") ui->labelAboutBootloader->setText("Рекомендуется для старых компьютеров. Текстовый");
}

void Window::on_deleteButtonMain_clicked()
{
    ui->progressDelete->setValue(0);
    ui->labelPlaceDeleteText->clear();
    ui->labelbootloaderDeleteText->clear();

    qDebug().noquote() << tr("Clearing systems list...");
    ui->comboSystemDelete->blockSignals(true);
    ui->comboSystemDelete->clear();
    ui->comboSystemDelete->blockSignals(false);

    qDebug().noquote() << tr("Filling systems list...");
    for(auto sys : global->insSet->systemsVector()) ui->comboSystemDelete->addItem(sys.name);

    setWindowTitle(tr("YourDroid | Delete"));
    ui->windows->setCurrentWidget(ui->deletePage);
}

void Window::on_comboSystemDelete_currentIndexChanged(int index)
{
    ui->labelPlaceDeleteText->setText(global->insSet->systemsVector()[index].place);
    ui->labelbootloaderDeleteText->setText(QString::fromStdString(_bootloaderHelper::to_string(global->insSet->systemsVector()[index].bootloader)));
}
void Window::on_buttonDeleteDelete_clicked()
{
    if(ui->comboSystemDelete->currentIndex() == -1) return;
    qDebug().noquote() << QString("^%1|+-|").arg(QObject::tr("Are you sure?"));
    auto choice = log::getLastPressedButton();
    if(choice == QMessageBox::Ok)
    {
        qDebug().noquote() << "Yes";
    }
    else
    {
        qDebug().noquote() << "No. Canceling";
        return;
    }

#if WIN
    taskBarProgress->resume();
#endif

    androidDelete();

    global->insSet->write();

    qDebug().noquote() << QObject::tr("^Finished");
}

void Window::androidDelete()
{
    ui->buttonDeleteDelete->setEnabled(false);
    ui->buttonReturnMainDelete->setEnabled(false);
    ui->settingsMini->setEnabled(false);
    ui->comboSystemDelete->setEnabled(false);
    int num = ui->comboSystemDelete->currentIndex();
    qDebug().noquote() << qApp->translate("log", "Deleting ") + (global->insSet->systemsVector().begin() + num)->name;
    global->insSet->delSystemFiles(num);
    global->insSet->deleteBootloaderEntry(num);
    global->insSet->oldSysEdit() = true;
    global->insSet->deleteSettingsEntry(num);
    on_deleteButtonMain_clicked();
    ui->statusbar->showMessage(tr("Ready"));
    ui->buttonDeleteDelete->setEnabled(true);
    ui->buttonReturnMainDelete->setEnabled(true);
    ui->settingsMini->setEnabled(true);
    ui->comboSystemDelete->setEnabled(true);
}

void Window::receiveMesToStatusbar(QString mes) {
    ui->statusbar->showMessage(mes);
}

void Window::closeEvent(QCloseEvent *event) {
    exiting = true;
    //emit closed();
    //event->accept();
}

void Window::changeEvent(QEvent *event) {
    //    if(event->type() == QEvent::WindowStateChange) emit deactived();
    //    else if(event->type() == QEvent::WindowActivate) emit actived();
    event->accept();
}

void Window::consoleHided() {
    if(exiting) return;
    global->set->setConEnable(false);
    global->set->autowrite_set();
    ui->checkEnableConSettings->setChecked(false);
}

void Window::on_comboLanguageSettings_currentIndexChanged(int index)
{
    langChanged = true;
}

#if WIN
void Window::setTaskProgress() {
    if(taskBarButton != nullptr) delete taskBarButton;
    taskBarButton = new QWinTaskbarButton(this);
    taskBarButton->setWindow(windowHandle());
    taskBarProgress = taskBarButton->progress();
    taskBarProgress->setVisible(true);
}
#endif

void Window::on_radioButton_clicked()
{
    //ui->comboFlashDrivesInstall->setEnabled(true);
    ui->editDirForInstall->setEnabled(false);
    ui->buttonChooseDirForInstall->setEnabled(false);
}

void Window::on_radioInstallOnDir_clicked()
{
    ui->comboBoot->setEnabled(true);
    //ui->comboFlashDrivesInstall->setEnabled(false);
    ui->editDirForInstall->setEnabled(true);
    ui->buttonChooseDirForInstall->setEnabled(true);
}

void Window::on_radioInstallFlashDriveIns_clicked()
{
    ui->comboBoot->setEnabled(false);
}

void Window::on_radioInstallOnPart_clicked()
{
    ui->comboBoot->setEnabled(true);
}
