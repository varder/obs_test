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


extern obs_frontend_callbacks *InitializeAPIInterface(OBSBasic *main);
static void AddExtraModulePaths()
{
    char base_module_dir[512];
#if defined(_WIN32) || defined(__APPLE__)
    int ret = GetProgramDataPath(base_module_dir, sizeof(base_module_dir),
            "obs-studio/plugins/%module%");
#else
    int ret = GetConfigPath(base_module_dir, sizeof(base_module_dir),
            "obs-studio/plugins/%module%");
#endif

    if (ret <= 0)
        return;

    std::string path = (char*)base_module_dir;
#if defined(__APPLE__)
    obs_add_module_path((path + "/bin").c_str(), (path + "/data").c_str());

    BPtr<char> config_bin = os_get_config_path_ptr("obs-studio/plugins/%module%/bin");
    BPtr<char> config_data = os_get_config_path_ptr("obs-studio/plugins/%module%/data");
    obs_add_module_path(config_bin, config_data);

#elif ARCH_BITS == 64
    obs_add_module_path((path + "/bin/64bit").c_str(),
            (path + "/data").c_str());
#else
    obs_add_module_path((path + "/bin/32bit").c_str(),
            (path + "/data").c_str());
#endif
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    ConfigFile                     globalConfig;
    ConfigFile    basicConfig;
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
    OBSService service;

    obs_frontend_callbacks *api = nullptr;

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

//            InitBasicConfigDefaults();

            obs_startup("en-US",  R"_(C:\Users\varder\AppData\Roaming\obs-studio/plugin_config)_", profilerNameStore);

            AddExtraModulePaths();
            blog(LOG_INFO, "--------------------------------- ALL MODULES");
            obs_load_all_modules();
            blog(LOG_INFO, "--------------------------------- LOG MODULES");
            obs_log_loaded_modules();
            blog(LOG_INFO, "--------------------------------- POST MODULES");
            obs_post_load_modules();
            blog(LOG_INFO, "--------------------------------- POST MODULES <<<<<<<<");

            blog(LOG_INFO, "STA=======================");

            bool isInitedService = InitService();
            qDebug() <<" servce Inited " << isInitedService;

            InitPrimitives();


//            connect(this->program, &OBSQTDisplay::DisplayCreated, addDisplay);

            auto addDisplay = [this] (OBSQTDisplay *window)
            {
                obs_display_add_draw_callback(window->GetDisplay(),
                        MainWindow::RenderMain, this);

                struct obs_video_info ovi;
                if (obs_get_video_info(&ovi)){

                    ResizePreview(ovi.base_width, ovi.base_height);
                    qDebug() <<"video Info ovi mainWin " << ovi.base_height << ovi.base_width << ovi.graphics_module;
                }
                qDebug() <<"video Info ovi mainWin " ;
            };


          CreateProgramDisplay();
          connect(program, &OBSQTDisplay::DisplayCreated, addDisplay);
          program->setGeometry(0,0, 500, 400);


    }


    static void RenderMain(void *data, uint32_t cx, uint32_t cy);



    bool InitService();

    void ResizePreview(uint32_t cx, uint32_t cy)
    {
    //	QSize  targetSize;
    //	bool isFixedScaling;
    //	obs_video_info ovi;

    //	/* resize preview panel to fix to the top section of the window */
    //	targetSize = GetPixelSize(ui->preview);

    //	isFixedScaling = ui->preview->IsFixedScaling();
    //	obs_get_video_info(&ovi);

    //	if (isFixedScaling) {
    //		previewScale = ui->preview->GetScalingAmount();
    //		GetCenterPosFromFixedScale(int(cx), int(cy),
    //				targetSize.width() - PREVIEW_EDGE_SIZE * 2,
    //				targetSize.height() - PREVIEW_EDGE_SIZE * 2,
    //				previewX, previewY, previewScale);
    //		previewX += ui->preview->GetScrollX();
    //		previewY += ui->preview->GetScrollY();

    //	} else {
    //		GetScaleAndCenterPos(int(cx), int(cy),
    //				targetSize.width() - PREVIEW_EDGE_SIZE * 2,
    //				targetSize.height() - PREVIEW_EDGE_SIZE * 2,
    //				previewX, previewY, previewScale);
    //	}

    //	previewX += float(PREVIEW_EDGE_SIZE);
    //	previewY += float(PREVIEW_EDGE_SIZE);
    }



    void InitPrimitives();
    bool InitGlobalConfig();
    void DrawBackdrop(float cx, float cy);
    void CreateProgramDisplay();
    void ResizeProgram(uint32_t cx, uint32_t cy);
    void GetConfigFPS(uint32_t &num, uint32_t &den) const
    {
        uint32_t type = config_get_uint(basicConfig, "Video", "FPSType");

        if (type == 1) //"Integer"
            GetFPSInteger(num, den);
        else if (type == 2) //"Fraction"
            GetFPSFraction(num, den);
        else if (false) //"Nanoseconds", currently not implemented
            GetFPSNanoseconds(num, den);
        else
            GetFPSCommon(num, den);
    }

    void GetFPSCommon(uint32_t &num, uint32_t &den) const
    {
        const char *val = config_get_string(basicConfig, "Video", "FPSCommon");

        if (strcmp(val, "10") == 0) {
            num = 10;
            den = 1;
        } else if (strcmp(val, "20") == 0) {
            num = 20;
            den = 1;
        } else if (strcmp(val, "24 NTSC") == 0) {
            num = 24000;
            den = 1001;
        } else if (strcmp(val, "25") == 0) {
            num = 25;
            den = 1;
        } else if (strcmp(val, "29.97") == 0) {
            num = 30000;
            den = 1001;
        } else if (strcmp(val, "48") == 0) {
            num = 48;
            den = 1;
        } else if (strcmp(val, "59.94") == 0) {
            num = 60000;
            den = 1001;
        } else if (strcmp(val, "60") == 0) {
            num = 60;
            den = 1;
        } else {
            num = 30;
            den = 1;
        }
    }

    void GetFPSInteger(uint32_t &num, uint32_t &den) const
    {
        num = (uint32_t)config_get_uint(basicConfig, "Video", "FPSInt");
        den = 1;
    }

    void GetFPSFraction(uint32_t &num, uint32_t &den) const
    {
        num = (uint32_t)config_get_uint(basicConfig, "Video", "FPSNum");
        den = (uint32_t)config_get_uint(basicConfig, "Video", "FPSDen");
    }

    void GetFPSNanoseconds(uint32_t &num, uint32_t &den) const
    {
        num = 1000000000;
        den = (uint32_t)config_get_uint(basicConfig, "Video", "FPSNS");
    }

    bool InitBasicConfigDefaults()
    {
        QList<QScreen*> screens = QGuiApplication::screens();

        if (!screens.size()) {
            OBSErrorBox(NULL, "There appears to be no monitors.  Er, this "
                              "technically shouldn't be possible.");
            return false;
        }

        QScreen *primaryScreen = QGuiApplication::primaryScreen();

        uint32_t cx = primaryScreen->size().width();
        uint32_t cy = primaryScreen->size().height();

        bool oldResolutionDefaults = config_get_bool(App()->GlobalConfig(),
                "General", "Pre19Defaults");

        /* use 1920x1080 for new default base res if main monitor is above
         * 1920x1080, but don't apply for people from older builds -- only to
         * new users */
        if (!oldResolutionDefaults && (cx * cy) > (1920 * 1080)) {
            cx = 1920;
            cy = 1080;
        }

        /* ----------------------------------------------------- */
        /* move over mixer values in advanced if older config */
        if (config_has_user_value(basicConfig, "AdvOut", "RecTrackIndex") &&
            !config_has_user_value(basicConfig, "AdvOut", "RecTracks")) {

            uint64_t track = config_get_uint(basicConfig, "AdvOut",
                    "RecTrackIndex");
            track = 1ULL << (track - 1);
            config_set_uint(basicConfig, "AdvOut", "RecTracks", track);
            config_remove_value(basicConfig, "AdvOut", "RecTrackIndex");
            config_save_safe(basicConfig, "tmp", nullptr);
        }

        /* ----------------------------------------------------- */

        config_set_default_string(basicConfig, "Output", "Mode", "Simple");

        config_set_default_string(basicConfig, "SimpleOutput", "FilePath",
                GetDefaultVideoSavePath().c_str());
        config_set_default_string(basicConfig, "SimpleOutput", "RecFormat",
                "flv");
        config_set_default_uint  (basicConfig, "SimpleOutput", "VBitrate",
                2500);
        config_set_default_string(basicConfig, "SimpleOutput", "StreamEncoder",
                SIMPLE_ENCODER_X264);
        config_set_default_uint  (basicConfig, "SimpleOutput", "ABitrate", 160);
        config_set_default_bool  (basicConfig, "SimpleOutput", "UseAdvanced",
                false);
        config_set_default_bool  (basicConfig, "SimpleOutput", "EnforceBitrate",
                true);
        config_set_default_string(basicConfig, "SimpleOutput", "Preset",
                "veryfast");
        config_set_default_string(basicConfig, "SimpleOutput", "RecQuality",
                "Stream");
        config_set_default_string(basicConfig, "SimpleOutput", "RecEncoder",
                SIMPLE_ENCODER_X264);
        config_set_default_bool(basicConfig, "SimpleOutput", "RecRB", false);
        config_set_default_int(basicConfig, "SimpleOutput", "RecRBTime", 20);
        config_set_default_int(basicConfig, "SimpleOutput", "RecRBSize", 512);
        config_set_default_string(basicConfig, "SimpleOutput", "RecRBPrefix",
                "Replay");

        config_set_default_bool  (basicConfig, "AdvOut", "ApplyServiceSettings",
                true);
        config_set_default_bool  (basicConfig, "AdvOut", "UseRescale", false);
        config_set_default_uint  (basicConfig, "AdvOut", "TrackIndex", 1);
        config_set_default_string(basicConfig, "AdvOut", "Encoder", "obs_x264");

        config_set_default_string(basicConfig, "AdvOut", "RecType", "Standard");

        config_set_default_string(basicConfig, "AdvOut", "RecFilePath",
                GetDefaultVideoSavePath().c_str());
        config_set_default_string(basicConfig, "AdvOut", "RecFormat", "flv");
        config_set_default_bool  (basicConfig, "AdvOut", "RecUseRescale",
                false);
        config_set_default_uint  (basicConfig, "AdvOut", "RecTracks", (1<<0));
        config_set_default_string(basicConfig, "AdvOut", "RecEncoder",
                "none");

        config_set_default_bool  (basicConfig, "AdvOut", "FFOutputToFile",
                true);
        config_set_default_string(basicConfig, "AdvOut", "FFFilePath",
                GetDefaultVideoSavePath().c_str());
        config_set_default_string(basicConfig, "AdvOut", "FFExtension", "mp4");
        config_set_default_uint  (basicConfig, "AdvOut", "FFVBitrate", 2500);
        config_set_default_uint  (basicConfig, "AdvOut", "FFVGOPSize", 250);
        config_set_default_bool  (basicConfig, "AdvOut", "FFUseRescale",
                false);
        config_set_default_bool  (basicConfig, "AdvOut", "FFIgnoreCompat",
                false);
        config_set_default_uint  (basicConfig, "AdvOut", "FFABitrate", 160);
        config_set_default_uint  (basicConfig, "AdvOut", "FFAudioTrack", 1);

        config_set_default_uint  (basicConfig, "AdvOut", "Track1Bitrate", 160);
        config_set_default_uint  (basicConfig, "AdvOut", "Track2Bitrate", 160);
        config_set_default_uint  (basicConfig, "AdvOut", "Track3Bitrate", 160);
        config_set_default_uint  (basicConfig, "AdvOut", "Track4Bitrate", 160);
        config_set_default_uint  (basicConfig, "AdvOut", "Track5Bitrate", 160);
        config_set_default_uint  (basicConfig, "AdvOut", "Track6Bitrate", 160);

        config_set_default_uint  (basicConfig, "Video", "BaseCX",   cx);
        config_set_default_uint  (basicConfig, "Video", "BaseCY",   cy);

        /* don't allow BaseCX/BaseCY to be susceptible to defaults changing */
        if (!config_has_user_value(basicConfig, "Video", "BaseCX") ||
            !config_has_user_value(basicConfig, "Video", "BaseCY")) {
            config_set_uint(basicConfig, "Video", "BaseCX", cx);
            config_set_uint(basicConfig, "Video", "BaseCY", cy);
            config_save_safe(basicConfig, "tmp", nullptr);
        }

        config_set_default_string(basicConfig, "Output", "FilenameFormatting",
                "%CCYY-%MM-%DD %hh-%mm-%ss");

        config_set_default_bool  (basicConfig, "Output", "DelayEnable", false);
        config_set_default_uint  (basicConfig, "Output", "DelaySec", 20);
        config_set_default_bool  (basicConfig, "Output", "DelayPreserve", true);

        config_set_default_bool  (basicConfig, "Output", "Reconnect", true);
        config_set_default_uint  (basicConfig, "Output", "RetryDelay", 10);
        config_set_default_uint  (basicConfig, "Output", "MaxRetries", 20);

        config_set_default_string(basicConfig, "Output", "BindIP", "default");
        config_set_default_bool  (basicConfig, "Output", "NewSocketLoopEnable",
                false);
        config_set_default_bool  (basicConfig, "Output", "LowLatencyEnable",
                false);

        int i = 0;
        uint32_t scale_cx = cx;
        uint32_t scale_cy = cy;

        /* use a default scaled resolution that has a pixel count no higher
         * than 1280x720 */
//        while (((scale_cx * scale_cy) > (1280 * 720)) && scaled_vals[i] > 0.0) {
//            double scale = scaled_vals[i++];
//            scale_cx = uint32_t(double(cx) / scale);
//            scale_cy = uint32_t(double(cy) / scale);
//        }

        config_set_default_uint  (basicConfig, "Video", "OutputCX", scale_cx);
        config_set_default_uint  (basicConfig, "Video", "OutputCY", scale_cy);

        /* don't allow OutputCX/OutputCY to be susceptible to defaults
         * changing */
        if (!config_has_user_value(basicConfig, "Video", "OutputCX") ||
            !config_has_user_value(basicConfig, "Video", "OutputCY")) {
            config_set_uint(basicConfig, "Video", "OutputCX", scale_cx);
            config_set_uint(basicConfig, "Video", "OutputCY", scale_cy);
            config_save_safe(basicConfig, "tmp", nullptr);
        }

        config_set_default_uint  (basicConfig, "Video", "FPSType", 0);
        config_set_default_string(basicConfig, "Video", "FPSCommon", "30");
        config_set_default_uint  (basicConfig, "Video", "FPSInt", 30);
        config_set_default_uint  (basicConfig, "Video", "FPSNum", 30);
        config_set_default_uint  (basicConfig, "Video", "FPSDen", 1);
        config_set_default_string(basicConfig, "Video", "ScaleType", "bicubic");
        config_set_default_string(basicConfig, "Video", "ColorFormat", "NV12");
        config_set_default_string(basicConfig, "Video", "ColorSpace", "601");
        config_set_default_string(basicConfig, "Video", "ColorRange",
                "Partial");

        config_set_default_string(basicConfig, "Audio", "MonitoringDeviceId",
                "default");
        config_set_default_string(basicConfig, "Audio", "MonitoringDeviceName",
                Str("Basic.Settings.Advanced.Audio.MonitoringDevice"
                    ".Default"));
        config_set_default_uint  (basicConfig, "Audio", "SampleRate", 44100);
        config_set_default_string(basicConfig, "Audio", "ChannelSetup",
                "Stereo");

        return true;
    }

    static void RenderProgram(void *data, uint32_t cx, uint32_t cy);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
