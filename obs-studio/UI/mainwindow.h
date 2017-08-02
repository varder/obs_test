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

#include <QBuffer>
#include <QAction>
#include <QSystemTrayIcon>
#include <obs.hpp>
#include <vector>
#include <memory>
#include "window-main.hpp"
#include "window-basic-interaction.hpp"
#include "window-basic-properties.hpp"
#include "window-basic-transform.hpp"
#include "window-basic-adv-audio.hpp"
#include "window-basic-filters.hpp"

#include <obs-frontend-internal.hpp>

#include <util/platform.h>
#include <util/threading.h>
#include <util/util.hpp>

#include <QPointer>
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

#ifdef _WIN32
#define IS_WIN32 1
#else
#define IS_WIN32 0
#endif

static inline int AttemptToResetVideo(struct obs_video_info *ovi)
{
    qDebug() << "struct obs_video_info"  << ovi;
    return obs_reset_video(ovi);
}


static inline enum obs_scale_type GetScaleType(ConfigFile &basicConfig)
{
    const char *scaleTypeStr = config_get_string(basicConfig,
            "Video", "ScaleType");

    if (astrcmpi(scaleTypeStr, "bilinear") == 0)
        return OBS_SCALE_BILINEAR;
    else if (astrcmpi(scaleTypeStr, "lanczos") == 0)
        return OBS_SCALE_LANCZOS;
    else
        return OBS_SCALE_BICUBIC;
}

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

static OBSWeakSource programScene;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    ConfigFile                     globalConfig;
    ConfigFile                     basicConfig;
    TextLookup                     textLookup;
    OBSContext                     obsContext;
    profiler_name_store_t         *profilerNameStore = nullptr;

    int   programX = 0,  programY = 0;
    float programScale = 1.0f;


//    gs_vertbuffer_t *box = nullptr;
//    gs_vertbuffer_t *boxLeft = nullptr;
//    gs_vertbuffer_t *boxTop = nullptr;
//    gs_vertbuffer_t *boxRight = nullptr;
//    gs_vertbuffer_t *boxBottom = nullptr;
//    gs_vertbuffer_t *circle = nullptr;

    QPointer<OBSQTDisplay> program;
    std::unique_ptr<BasicOutputHandler> outputHandler;
    OBSService service;

//    obs_frontend_callbacks *api = nullptr;

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void AppInit(){

//            ProfileScope("OBSApp::AppInit");
            AddExtraModulePaths();

            if (!InitApplicationBundle())
                throw "Failed to initialize application bundle";
//            if (!MakeUserDirs())
//                throw "Failed to create required user directories";
            if (!InitGlobalConfig())
                throw "Failed to initialize global config";


            obs_startup("en-US",  R"_(C:\Users\varder\AppData\Roaming\obs-studio/plugin_config)_", profilerNameStore);

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
             ResetVideo();


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
          program->setGeometry(0,0, 800, 500);

          InitPrimitives();

          Load1("");

    }

    int ResetVideo()
    {
        if (outputHandler && outputHandler->Active())
            return OBS_VIDEO_CURRENTLY_ACTIVE;
        ProfileScope("OBSBasic::ResetVideo");

        struct obs_video_info ovi;
        int ret;

        ovi.fps_num = 30;
        ovi.fps_den = 1;
        ovi.graphics_module = DL_D3D11 ; //App()->GetRenderModule();  "DL_D3D11=\"libobs-d3d11.dll\"
        ovi.base_width     = (uint32_t)2560;
        ovi.base_height    = (uint32_t)1440;
        ovi.output_width   = (uint32_t)1280;
        ovi.output_height  = (uint32_t)720;
        ovi.output_format  = VIDEO_FORMAT_I420; //GetVideoFormatFromName(colorFormat);
        ovi.colorspace     = VIDEO_CS_709 ; // VIDEO_CS_601 : VIDEO_CS_709;
        ovi.range          = VIDEO_RANGE_FULL; // VIDEO_RANGE_FULL : VIDEO_RANGE_PARTIAL;
        ovi.adapter        = 0;
        ovi.gpu_conversion = true;
        ovi.scale_type     = OBS_SCALE_BILINEAR;// GetScaleType(basicConfig);

        if (ovi.base_width == 0 || ovi.base_height == 0) {
            ovi.base_width = 1920;
            ovi.base_height = 1080;
            config_set_uint(basicConfig, "Video", "BaseCX", 1920);
            config_set_uint(basicConfig, "Video", "BaseCY", 1080);
        }

        if (ovi.output_width == 0 || ovi.output_height == 0) {
            ovi.output_width = ovi.base_width;
            ovi.output_height = ovi.base_height;
            config_set_uint(basicConfig, "Video", "OutputCX",
                    ovi.base_width);
            config_set_uint(basicConfig, "Video", "OutputCY",
                    ovi.base_height);
        }

        ret = AttemptToResetVideo(&ovi);
        qDebug() << "retttt " << ret;
        if (IS_WIN32 && ret != OBS_VIDEO_SUCCESS) {
            if (ret == OBS_VIDEO_CURRENTLY_ACTIVE) {
                blog(LOG_WARNING, "Tried to reset when "
                                  "already active");
                return ret;
            }

            /* Try OpenGL if DirectX fails on windows */
            if (astrcmpi(ovi.graphics_module, DL_OPENGL) != 0) {
                blog(LOG_WARNING, "Failed to initialize obs video (%d) "
                          "with graphics_module='%s', retrying "
                          "with graphics_module='%s'",
                          ret, ovi.graphics_module,
                          DL_OPENGL);
                ovi.graphics_module = DL_OPENGL;
                ret = AttemptToResetVideo(&ovi);
            }
        } else if (ret == OBS_VIDEO_SUCCESS) {
            ResizePreview(ovi.base_width, ovi.base_height);
            if (program)
                ResizeProgram(ovi.base_width, ovi.base_height);
        }

        if (ret == OBS_VIDEO_SUCCESS)
            OBSBasicStats::InitializeValues();
        qDebug() << "retttt222 " << ret;
//        return 0;

        return ret;
    }

    void Load1(const char *file1)
    {

        const char *file = R"_(C:\Users\v.chubar\AppData\Roaming\obs-studio/basic/scenes/varder.json)_";
//        const char *file = R"_(C:\Users\varder\AppData\Roaming\obs-studio/basic/scenes/varder.json)_";
        obs_data_t *data = nullptr;
        qDebug() << "before loaded file " << !!file;
        data = obs_data_create_from_json_file_safe(file, "bak");
        if(!data){
            return;
        }
        const char *sceneName = obs_data_get_string(data, "current_scene");
        obs_data_array_t *sources    = obs_data_get_array(data, "sources");

        obs_load_sources(sources, MainWindow::SourceLoaded, this);

        obs_source_t     *curScene = nullptr;
        curScene = obs_get_source_by_name(sceneName);

        qDebug() << "loaded curr scene  " << sceneName;

    }

    static void SourceLoaded(void *data, obs_source_t *source)
    {
       obs_scene_t *scene = obs_scene_from_source(source);
       obs_source_get_name(source);
       if(strcmp(obs_source_get_name(source), "scene4")==0){
            obs_source_inc_showing(source);
            qDebug () << "got scdene " << obs_source_get_name(source) ;//<< !!MAIN_SCENE;
            obs_scene_addref(scene);
            programScene = OBSGetWeakRef(source);
       }
       if(obs_source_active(obs_scene_get_source(scene) )){
           qDebug() << "source is active " ;
       }
        qDebug() <<"source loaded " << !!scene << " scene name " << obs_source_get_name(source);
    }

    static void RenderMain(void *data, uint32_t cx, uint32_t cy)
    {//return;

        obs_video_info ovi;

        obs_get_video_info(&ovi);


        gs_viewport_push();
        gs_projection_push();

        /* --------------------------------------- */

        gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height),
                 -100.0f, 100.0f);
        //    gs_set_viewport(0, 0, 500, 400);

        gs_set_viewport(73, 10, 522, 294);

        DrawBackdrop(float(ovi.base_width), float(ovi.base_height));

        if(programScene){
            OBSSource src  = OBSGetStrongRef( programScene);
//            qDebug() << "source valied " << !!src << obs_source_get_name(src);//
             obs_source_video_render(src);
        }

//        obs_render_main_view();


        gs_load_vertexbuffer(nullptr);

        /* --------------------------------------- */


        gs_ortho(-73.f,  596.f,
                 -10.f,  304.f,
                 -100.f, 100.f);
        //    gs_ortho(-window->previewX, right,
        //             -window->previewY, bottom,
        //             -100.0f, 100.0f);
        gs_reset_viewport();

        //    window->ui->preview->DrawSceneEditing();

        /* --------------------------------------- */

        gs_projection_pop();
        gs_viewport_pop();

        UNUSED_PARAMETER(cx);
        UNUSED_PARAMETER(cy);
    }


    bool InitService();

    void ResizePreview(uint32_t cx, uint32_t cy)
    {
    }

    void InitPrimitives();
    bool InitGlobalConfig();
    static void DrawBackdrop(float cx, float cy);
    void CreateProgramDisplay();
    void ResizeProgram(uint32_t cx, uint32_t cy);
    static void RenderProgram(void *data, uint32_t cx, uint32_t cy);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
