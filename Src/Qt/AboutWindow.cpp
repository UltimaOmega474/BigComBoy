#include "AboutWindow.hpp"
#include "ui_AboutWindow.h"

namespace QtFrontend {

    AboutWindow::AboutWindow(QWidget *parent) : QDialog(parent), ui(new Ui::AboutWindow) {
        ui->setupUi(this);
        setFixedSize(sizeHint());
        setAttribute(Qt::WA_DeleteOnClose);

        QString img = "<img src=\":/bcb_icon/bcb.png\" width=\"64\"height=\"64\" "
                      "style=\"vertical-align:bottom\"/>";

        QString title = " Big ComBoy";

        QString version = "Version: " BCB_VER;
        QString description = "Open source Game Boy & Game Boy Color emulator.<br>";
        QString build = QString("Build Date: %1 %2").arg(__DATE__).arg(__TIME__);

        ui->desc_text->setText(
            ui->desc_text->text().arg(img).arg(title).arg(description).arg(version).arg(build));
    }

    AboutWindow::~AboutWindow() {
        delete ui;
        ui = nullptr;
    }

}