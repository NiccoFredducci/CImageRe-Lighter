#include "raylib.h"
#include "pipeline.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800
#define SIDEBAR_WIDTH 260

typedef struct App {
    Image original;
    Texture2D preview;
    PipelineOutputs outputs;

    bool image_loaded;
    bool path_focused;
    bool dropdown_open;

    char image_path[512];
    char status[256];

    OutputKind selected_output;
} App;


// --------------------------------------------------
// UI HELPERS
// --------------------------------------------------

//static const Color BG       = {235, 237, 239, 255};
//static const Color PANEL    = {247, 248, 249, 255};
//static const Color BORDER   = {190, 195, 200, 255};
//static const Color TEXT     = {42, 47, 52, 255};
//static const Color MUTED    = {105, 112, 119, 255};
//static const Color WHITE    = {255, 255, 255, 255};

// UI colors
static const Color BG     = {245, 245, 245, 255};
static const Color PANEL  = {235, 235, 235, 255};
static const Color BORDER = {205, 205, 205, 255};
static const Color TEXT   = {35, 35, 35, 255};
static const Color MUTED  = {120, 120, 120, 255};

static Font uiFont;

void DrawUIText(const char *text, int x, int y, int size, Color color)
{
    DrawTextEx(
        uiFont,
        text,
        (Vector2){(float)x, (float)y},
        (float)size,
        1.0f,
        color
    );
}

static bool mouse_over(Rectangle r)
{
    return CheckCollisionPointRec(GetMousePosition(), r);
}


static bool button(Rectangle r, const char *label)
{
    bool hover = mouse_over(r);

    DrawRectangleRec(r, hover ? BG : PANEL);
    DrawRectangleLinesEx(r, 1, BORDER);

    DrawUIText(label,
             r.x + 12,
             r.y + 12,
             16,
             TEXT);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}


static void draw_label(const char *label, int x, int y)
{
    DrawUIText(label, x, y, 12, MUTED);
}


// --------------------------------------------------
// IMAGE HANDLING
// --------------------------------------------------

static void load_image(App *app)
{
	TraceLog(LOG_INFO, "Attempting to load image: '%s'", app->image_path);
    Image candidate = LoadImage(app->image_path);

    if (candidate.data == NULL) {
        snprintf(app->status, sizeof(app->status),
                 "Failed to load image.");
        return;
    }

    if (app->image_loaded) {
        UnloadTexture(app->preview);
        UnloadImage(app->original);
        pipeline_unload(&app->outputs);
    }

    app->original = candidate;

    pipeline_run(app->original, &app->outputs);

    app->preview = LoadTextureFromImage(app->original);

    app->image_loaded = true;
    app->selected_output = OUTPUT_ORIGINAL;

    snprintf(app->status, sizeof(app->status),
             "Image loaded: %dx%d",
             app->original.width,
             app->original.height);
}


static void update_preview(App *app)
{
    if (!app->image_loaded) return;

    Image current = pipeline_get_image(
        &app->outputs,
        app->selected_output
    );

    if (app->preview.id != 0)
        UnloadTexture(app->preview);

    app->preview = LoadTextureFromImage(current);
}


static void save_current_image(App *app)
{
    if (!app->image_loaded) return;

    Image current = pipeline_get_image(
        &app->outputs,
        app->selected_output
    );

    if (ExportImage(current, "relight_output.png")) {
        snprintf(app->status, sizeof(app->status),
                 "Image saved successfully.");
    } else {
        snprintf(app->status, sizeof(app->status),
                 "Failed to save image.");
    }
}


// --------------------------------------------------
// IMAGE DISPLAY
// --------------------------------------------------

static void draw_image(App *app, Rectangle area)
{
    if (!app->image_loaded) {
        DrawUIText("No image loaded",
                 area.x + 30,
                 area.y + 30,
                 20,
                 MUTED);
        return;
    }

    float scale_x = area.width / app->preview.width;
    float scale_y = area.height / app->preview.height;

    float scale = scale_x < scale_y ? scale_x : scale_y;

    float width = app->preview.width * scale;
    float height = app->preview.height * scale;

    Rectangle source = {
        0, 0,
        app->preview.width,
        app->preview.height
    };

    Rectangle destination = {
        area.x + (area.width - width) / 2,
        area.y + (area.height - height) / 2,
        width,
        height
    };

    DrawTexturePro(
        app->preview,
        source,
        destination,
        (Vector2){0, 0},
        0,
        WHITE
    );
}


// --------------------------------------------------
// OUTPUT DROPDOWN
// --------------------------------------------------

static void draw_output_dropdown(App *app, Rectangle r)
{
    if (button(r,
        pipeline_output_name(app->selected_output))) {

        app->dropdown_open = !app->dropdown_open;
    }

    if (!app->dropdown_open) return;

    for (int i = 0; i < OUTPUT_COUNT; i++) {

        Rectangle item = {
            r.x,
            r.y + r.height + i * 34,
            r.width,
            34
        };

        DrawRectangleRec(
            item,
            mouse_over(item) ? BG : WHITE
        );

        DrawRectangleLinesEx(item, 1, BORDER);

        DrawUIText(
            pipeline_output_name(i),
            item.x + 12,
            item.y + 9,
            13,
            TEXT
        );

        if (mouse_over(item) &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

            app->selected_output = i;
            app->dropdown_open = false;

            update_preview(app);
        }
    }
}


// --------------------------------------------------
// MAIN UI
// --------------------------------------------------

static void draw_ui(App *app)
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    Rectangle sidebar = {
        0, 0,
        SIDEBAR_WIDTH,
        screen_height
    };

    Rectangle viewer = {
        SIDEBAR_WIDTH + 18,
        18,
        screen_width - SIDEBAR_WIDTH - 36,
        screen_height - 36
    };

    Rectangle path_box = {
        18, 120,
        SIDEBAR_WIDTH - 36,
        38
    };

    Rectangle load_button = {
        18, 170,
        SIDEBAR_WIDTH - 36,
        42
    };

    Rectangle dropdown = {
        screen_width - 250,
        82,
        220,
        38
    };

    Rectangle image_area = {
        viewer.x + 22,
        140,
        viewer.width - 44,
        viewer.height - 180
    };

    Rectangle save_button = {
        screen_width - 190,
        screen_height - 64,
        160,
        42
    };


    // BACKGROUND

    ClearBackground(BG);

    DrawRectangleRec(sidebar, PANEL);

    DrawLine(
        SIDEBAR_WIDTH, 0,
        SIDEBAR_WIDTH, screen_height,
        BORDER
    );


    // SIDEBAR HEADER

    DrawUIText("RELIGHT", 18, 22, 23, TEXT);
    DrawUIText("IMAGE PIPELINE", 19, 48, 11, MUTED);

    DrawLine(18, 82, SIDEBAR_WIDTH - 18, 82, BORDER);


    // IMAGE INPUT

    draw_label("INPUT IMAGE", 18, 98);

    DrawRectangleRec(path_box, WHITE);
    DrawRectangleLinesEx(
        path_box,
        1,
        app->path_focused ? TEXT : BORDER
    );

    DrawUIText(
        app->image_path[0] ?
            app->image_path :
            "Enter image path...",
        path_box.x + 8,
        path_box.y + 11,
        12,
        TEXT
    );


    // LOAD BUTTON

    if (button(load_button, "LOAD IMAGE")) {
        load_image(app);
    }


    // PIPELINE INFORMATION

    DrawLine(18, 238, SIDEBAR_WIDTH - 18, 238, BORDER);

    draw_label("PIPELINE", 18, 254);

    const char *steps[] = {
        "01  Depth estimation",
        "02  Surface normals",
        "03  Albedo",
        "04  Roughness",
        "05  Metallic",
        "06  Neutral materials",
        "07  Re-lighting"
    };

    for (int i = 0; i < 7; i++) {
        DrawUIText(steps[i], 20, 284 + i * 31, 13, MUTED);
    }


    // STATUS

    DrawLine(
        18, screen_height - 88,
        SIDEBAR_WIDTH - 18, screen_height - 88,
        BORDER
    );

    draw_label("STATUS", 18, screen_height - 72);

    DrawUIText(
        app->status,
        18,
        screen_height - 52,
        10,
        MUTED
    );


    // VIEWER

    DrawRectangleRec(viewer, PANEL);
    DrawRectangleLinesEx(viewer, 1, BORDER);

    draw_label("VIEWER", viewer.x + 18, 32);


    // OUTPUT SELECTOR

    draw_output_dropdown(app, dropdown);


    // IMAGE

    draw_image(app, image_area);


    // SAVE BUTTON

    if (button(save_button, "SAVE VIEW")) {
        save_current_image(app);
    }
}

// --------------------------------------------------
// APPLICATION ENTRY POINT
// --------------------------------------------------

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Relight Studio"
    );
	
	TraceLog(LOG_INFO, "Attempting to load font: assets/fonts/latinmodern.ttf");
	uiFont = LoadFontEx("assets/fonts/cmunrm.ttf", 32, NULL, 0);
	
	if (uiFont.texture.id == 0)
	{
		TraceLog(LOG_ERROR, "Could not load UI font!");
		CloseWindow();
		return 1;
	}
	
	SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);


    SetTargetFPS(60);

    App app = {0};

    snprintf(
        app.status,
        sizeof(app.status),
        "Waiting for image..."
    );

    while (!WindowShouldClose()) {

        // PATH FIELD INPUT

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

            Rectangle path_box = {
                18, 120,
                SIDEBAR_WIDTH - 36,
                38
            };

            app.path_focused =
                mouse_over(path_box);
        }

        if (app.path_focused) {

            int key = GetCharPressed();

            while (key > 0) {

                if (key >= 32 && key <= 126) {

                    size_t len = strlen(app.image_path);

                    if (len < sizeof(app.image_path) - 1) {
                        app.image_path[len] = key;
                        app.image_path[len + 1] = '\0';
                    }
                }

                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                size_t len = strlen(app.image_path);

                if (len > 0)
                    app.image_path[len - 1] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER))
                app.path_focused = false;
        }


        BeginDrawing();

        draw_ui(&app);

        EndDrawing();
    }


    // CLEANUP

    if (app.image_loaded) {
        UnloadTexture(app.preview);
        UnloadImage(app.original);
        pipeline_unload(&app.outputs);
    }

    CloseWindow();

    return 0;
}
