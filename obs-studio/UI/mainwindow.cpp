#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


gs_vertbuffer_t *box = nullptr;
gs_vertbuffer_t *boxLeft = nullptr;
gs_vertbuffer_t *boxTop = nullptr;
gs_vertbuffer_t *boxRight = nullptr;
gs_vertbuffer_t *boxBottom = nullptr;
gs_vertbuffer_t *circle = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    profilerNameStore(nullptr)
{
    ui->setupUi(this);

//    TrackEditor* trackEditor = new TrackEditor;
    QVBoxLayout *layout = new QVBoxLayout;

//    layout->addWidget(trackEditor);
//    setLayout(layout);

//    QPushButton *m_button = new QPushButton("My Button", this);
//    // устанавливаем размер и положение кнопки
//    m_button->setGeometry(QRect(QPoint(100, 100),
//    QSize(200, 50)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RenderMain(void *data, uint32_t cx, uint32_t cy)
{//return;
    //    OBSBasic *window = static_cast<OBSBasic*>(data);
    obs_video_info ovi;

    obs_get_video_info(&ovi);

    //    window->previewCX = int(window->previewScale * float(ovi.base_width));
    //    window->previewCY = int(window->previewScale * float(ovi.base_height));

    gs_viewport_push();
    gs_projection_push();

    /* --------------------------------------- */

    gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height),
             -100.0f, 100.0f);
    //    gs_set_viewport(0, 0, 500, 400);

    gs_set_viewport(73, 10, 522, 294);

    //    window->DrawBackdrop(float(ovi.base_width), float(ovi.base_height));
    DrawBackdrop(float(ovi.base_width), float(ovi.base_height));

    //    if (window->IsPreviewProgramMode()) {
    //        OBSScene scene = window->GetCurrentScene();
    //        obs_source_t *source = obs_scene_get_source(scene);
    //        if (source);
    //                obs_source_video_render(source);
    //    } else {
    //        obs_render_main_view();
    //    }
    //        if(MAIN_SCENE){
    //            obs_source_video_render(MAIN_SCENE);
    //        }

    qDebug() <<  " Render main " << MAIN_SCENE;
    gs_load_vertexbuffer(nullptr);

    /* --------------------------------------- */

    //    QSize previewSize = QSize(100, 100);//GetPixelSize(window->ui->preview);
    //    float right  = float(previewSize.width())  - window->previewX;
    //    float bottom = float(previewSize.height()) - window->previewY;

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

//MainWindow::MAIN_SCENE = nullptr;





bool MainWindow::InitService()
{
    ProfileScope("OBSBasic::InitService");

    service = obs_service_create("rtmp_common", "default_service", nullptr,
                                 nullptr);
    if (!service)
        return false;
    obs_service_release(service);

    return true;
}

void MainWindow::InitPrimitives(){
    ProfileScope("OBSBasic::InitPrimitives");

    obs_enter_graphics();

    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(0.0f, 1.0f);
    gs_vertex2f(1.0f, 1.0f);
    gs_vertex2f(1.0f, 0.0f);
    gs_vertex2f(0.0f, 0.0f);
    box = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(0.0f, 1.0f);
    boxLeft = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(1.0f, 0.0f);
    boxTop = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(1.0f, 0.0f);
    gs_vertex2f(1.0f, 1.0f);
    boxRight = gs_render_save();

    gs_render_start(true);
    gs_vertex2f(0.0f, 1.0f);
    gs_vertex2f(1.0f, 1.0f);
    boxBottom = gs_render_save();

    gs_render_start(true);
    for (int i = 0; i <= 360; i += (360/20)) {
        float pos = RAD(float(i));
        gs_vertex2f(cosf(pos), sinf(pos));
    }
    circle = gs_render_save();

    obs_leave_graphics();
}
static bool do_mkdir(const char *path)
{
    if (os_mkdirs(path) == MKDIR_ERROR) {
        OBSErrorBox(NULL, "Failed to create directory %s", path);
        return false;
    }

    return true;
}

bool MainWindow::InitGlobalConfig()
{

    char path[512];

    if (GetConfigPath(path, sizeof(path), "obs-studio/basic") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;

    if (GetConfigPath(path, sizeof(path), "obs-studio/logs") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;

    if (GetConfigPath(path, sizeof(path), "obs-studio/profiler_data") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;

#ifdef _WIN32
    if (GetConfigPath(path, sizeof(path), "obs-studio/crashes") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;

    if (GetConfigPath(path, sizeof(path), "obs-studio/updates") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
#endif

    if (GetConfigPath(path, sizeof(path), "obs-studio/plugin_config") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;

    return true;
}

void MainWindow::DrawBackdrop(float cx, float cy)
{
    if (!box)
        return;

    gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
    gs_eparam_t    *color = gs_effect_get_param_by_name(solid, "color");
    gs_technique_t *tech  = gs_effect_get_technique(solid, "Solid");

    vec4 colorVal;
    vec4_set(&colorVal, 0.0f, 0.0f, 0.0f, 1.0f);
    gs_effect_set_vec4(color, &colorVal);

    gs_technique_begin(tech);
    gs_technique_begin_pass(tech, 0);
    gs_matrix_push();
    gs_matrix_identity();
    gs_matrix_scale3f(float(cx), float(cy), 1.0f);

    gs_load_vertexbuffer(box);
    gs_draw(GS_TRISTRIP, 0, 0);

    gs_matrix_pop();
    gs_technique_end_pass(tech);
    gs_technique_end(tech);

    gs_load_vertexbuffer(nullptr);
}

void MainWindow::CreateProgramDisplay()
{
    program = new OBSQTDisplay(this);

    auto displayResize = [this]() {
        struct obs_video_info ovi;

        if (obs_get_video_info(&ovi))
            ResizeProgram(ovi.base_width, ovi.base_height);
    };

    connect(program.data(), &OBSQTDisplay::DisplayResized,
            displayResize);

    auto addDisplay = [this] (OBSQTDisplay *window)
    {
        obs_display_add_draw_callback(window->GetDisplay(),
                                      MainWindow::RenderProgram, this);

        struct obs_video_info ovi;
        if (obs_get_video_info(&ovi))
            ResizeProgram(ovi.base_width, ovi.base_height);
        qDebug() <<" add DIspley callback ";
    };

    connect(program.data(), &OBSQTDisplay::DisplayCreated, addDisplay);

    program->setSizePolicy(QSizePolicy::Expanding,
                           QSizePolicy::Expanding);
    qDebug() << "display created ";
}

void MainWindow::ResizeProgram(uint32_t cx, uint32_t cy)
{
    QSize targetSize;

    /* resize program panel to fix to the top section of the window */
    targetSize = GetPixelSize(program);
        GetScaleAndCenterPos(int(cx), int(cy),
                             targetSize.width()  - PREVIEW_EDGE_SIZE * 2,
                             targetSize.height() - PREVIEW_EDGE_SIZE * 2,
                             programX, programY, programScale);

        programX += float(PREVIEW_EDGE_SIZE);
        programY += float(PREVIEW_EDGE_SIZE);
    qDebug() <<"resize programm ";
}

void MainWindow::RenderProgram(void *data, uint32_t cx, uint32_t cy)
{
//    OBSBasic *window = static_cast<OBSBasic*>(data);
    obs_video_info ovi;

    obs_get_video_info(&ovi);

    //    window->programCX = int(window->programScale * float(ovi.base_width));
    //    window->programCY = int(window->programScale * float(ovi.base_height));

    gs_viewport_push();
    gs_projection_push();

    /* --------------------------------------- */

    gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height),
             -100.0f, 100.0f);
//    gs_set_viewport(window->programX, window->programY,
//                    window->programCX, window->programCY);

//    window->DrawBackdrop(float(ovi.base_width), float(ovi.base_height));
    qDebug() << "reder ";
    obs_render_main_view();
    gs_load_vertexbuffer(nullptr);

    /* --------------------------------------- */

    gs_projection_pop();
    gs_viewport_pop();

    UNUSED_PARAMETER(cx);
    UNUSED_PARAMETER(cy);
}
