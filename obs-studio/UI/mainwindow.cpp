#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    profilerNameStore(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
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
    program = new OBSQTDisplay();

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
    //    GetScaleAndCenterPos(int(cx), int(cy),
    //                         targetSize.width()  - PREVIEW_EDGE_SIZE * 2,
    //                         targetSize.height() - PREVIEW_EDGE_SIZE * 2,
    //                         programX, programY, programScale);

    //    programX += float(PREVIEW_EDGE_SIZE);
    //    programY += float(PREVIEW_EDGE_SIZE);
}
#include <QDebug>
void MainWindow::RenderProgram(void *data, uint32_t cx, uint32_t cy)
{
    OBSBasic *window = static_cast<OBSBasic*>(data);
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
