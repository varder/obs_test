#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QApplication>
#include <QTranslator>
#include <QPointer>
#include <obs.hpp>
#include <util/lexer.h>
#include <util/profiler.h>
#include <util/util.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <time.h>
#include <stdio.h>
#include <wchar.h>
#include <chrono>
#include <ratio>
#include <string>
#include <sstream>
#include <mutex>
#include <util/bmem.h>
#include <util/dstr.h>
#include <util/platform.h>
#include <util/profiler.hpp>
#include <obs-config.h>
#include <obs.hpp>

#include <QGuiApplication>
#include <QProxyStyle>
#include <QScreen>

#include "qt-wrappers.hpp"
#include "obs-app.hpp"
#include "window-basic-main.hpp"
#include "window-basic-settings.hpp"
#include "window-license-agreement.hpp"
#include "crash-report.hpp"
#include "platform.hpp"

#include <time.h>
#include <obs.hpp>
#include <QGuiApplication>
#include <QMessageBox>
#include <QShowEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QRect>
#include <QScreen>

#include <util/dstr.h>
#include <util/util.hpp>
#include <util/platform.h>
#include <util/profiler.hpp>
#include <util/dstr.hpp>
#include <graphics/math-defs.h>

#include "obs-app.hpp"
#include "platform.hpp"
#include "visibility-item-widget.hpp"
#include "item-widget-helpers.hpp"
#include "window-basic-settings.hpp"
#include "window-namedialog.hpp"
#include "window-basic-auto-config.hpp"
#include "window-basic-source-select.hpp"
#include "window-basic-main.hpp"
#include "window-basic-stats.hpp"
#include "window-basic-main-outputs.hpp"
#include "window-basic-properties.hpp"
#include "window-log-reply.hpp"
#include "window-projector.hpp"
#include "window-remux.hpp"
#include "qt-wrappers.hpp"
#include "display-helpers.hpp"
#include "volume-control.hpp"
#include "remote-text.hpp"

#include <fstream>

#include <curl/curl.h>

#include "window-main.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    ConfigFile                     globalConfig;
    TextLookup                     textLookup;
    OBSContext                     obsContext;
    profiler_name_store_t          *profilerNameStore = nullptr;

    int   programX = 0,  programY = 0;
    float programScale = 1.0f;

    gs_vertbuffer_t *box = nullptr;
    gs_vertbuffer_t *boxLeft = nullptr;
    gs_vertbuffer_t *boxTop = nullptr;
    gs_vertbuffer_t *boxRight = nullptr;
    gs_vertbuffer_t *boxBottom = nullptr;
    gs_vertbuffer_t *circle = nullptr;

    QPointer<OBSQTDisplay> program;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void AppInit(){

            ProfileScope("OBSApp::AppInit");

            if (!InitApplicationBundle())
                throw "Failed to initialize application bundle";
//            if (!MakeUserDirs())
//                throw "Failed to create required user directories";
            if (!InitGlobalConfig())
                throw "Failed to initialize global config";

            obs_startup("ru-RU",  R"_(C:\Users\varder\AppData\Roaming\obs-studio/plugin_config)_", profilerNameStore);

            CreateProgramDisplay();

//            InitPrimitives();
//            program->CreateDisplay();

//            QPushButton *m_button = new QPushButton("My Button");
            QPushButton *m_button = new QPushButton("My Button");
            program->setGeometry(0,0, 500, 400);

            InitPrimitives();
            DrawBackdrop(50, 50);
            // устанавливаем размер и положение кнопки
//            m_button->setGeometry(QRect(QPoint(100, 100),
//            QSize(200, 50)));

//            QGridLayout *layout = new QGridLayout(this);

//            layout->addWidget(program,0, 0);
//            setLayout(layout);


    }


    void InitPrimitives();

    bool InitGlobalConfig();

    void DrawBackdrop(float cx, float cy);

    void CreateProgramDisplay();

    void ResizeProgram(uint32_t cx, uint32_t cy);

    static void RenderProgram(void *data, uint32_t cx, uint32_t cy);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
