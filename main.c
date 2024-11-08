#include "include/raylib.h"
#include "include/raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BACKGROUND                                                             \
  (Color) { 0x18, 0x18, 0x18, 255 }
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define FPS_TARGET 60

#define PARTICLE_COUNT 2000
#define PARTICLE_SIZE 3
#define GRAVITY 0.1
#define SPAWN_DELAY 10
#define COLOR_VARIATION 40
#define COLOR_SET_SIZE 5

typedef struct
{
  Vector2 position;
  Vector2 velocity;
  Color color;
  int lifespan;
  int max_lifespan;
} Particle;

Particle *particles = 0;
int particleCount = 0;

void add_particle(Vector2 position, Vector2 velocity, int lifespan, Color color)
{
  particles = realloc(particles, sizeof(Particle) * (particleCount + 1));
  particles[particleCount++] =
      (Particle){position, velocity, color, lifespan, lifespan};
}

float normalize(int value, int min_input, int max_input)
{
  return (max_input - value) / (float)(max_input - min_input);
}
Color random_color()
{
  return (Color){GetRandomValue(0, 255), GetRandomValue(0, 255),
                 GetRandomValue(0, 255), 255};
}

Color get_complementary_color(Color color)
{
  Color complementary;
  complementary.r = 255 - color.r;
  complementary.g = 255 - color.g;
  complementary.b = 255 - color.b;
  complementary.a = color.a;
  return complementary;
}

void generate_color_set(int count, Color colors[])
{
  Color base_color = random_color();
  Color complementary = get_complementary_color(base_color);

  for (int i = 0; i < count; i++)
  {
    colors[i].r = GetRandomValue(base_color.r - COLOR_VARIATION,
                                 base_color.r + COLOR_VARIATION);
    colors[i].g = GetRandomValue(base_color.g - COLOR_VARIATION,
                                 base_color.g + COLOR_VARIATION);
    colors[i].b = GetRandomValue(base_color.b - COLOR_VARIATION,
                                 base_color.b + COLOR_VARIATION);
    colors[i].a = base_color.a;

    // Ensure colors stay within the 0–255 range
    if (colors[i].r < 0)
      colors[i].r = 0;
    if (colors[i].g < 0)
      colors[i].g = 0;
    if (colors[i].b < 0)
      colors[i].b = 0;
    if (colors[i].r > 255)
      colors[i].r = 255;
    if (colors[i].g > 255)
      colors[i].g = 255;
    if (colors[i].b > 255)
      colors[i].b = 255;
  }

  // Generate complementary color variations
  Color complementary_colors[count];
  for (int i = 0; i < count; i++)
  {
    complementary_colors[i] = get_complementary_color(colors[i]);
  }

  // Combine base colors and complementary colors into one set
  for (int i = 0; i < count; i++)
  {
    colors[count + i] = complementary_colors[i];
  }
}

int main()
{
  srand(time(0));
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib | Fireworks");
  SetTargetFPS(FPS_TARGET);

  // Load Glow Shader
  Shader glowShader = LoadShader(0, "glow.fs");
  if (!glowShader.id)
  {
    printf("Shader failed to load.\n");
    return 1; // Exit if shader fails
  }

  int resolutionLoc = GetShaderLocation(glowShader, "resolution");
  Vector2 resolution = {(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};
  SetShaderValue(glowShader, resolutionLoc, &resolution, 4);

  int radiusLoc = GetShaderLocation(glowShader, "radius");
  float radius = 10.0f;
  SetShaderValue(glowShader, radiusLoc, &radius, 1);

  // Create a RenderTexture2D to store the fireworks' particle render
  RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

  int delay_frames = 0;

  while (!WindowShouldClose())
  {
    // Update
    if (delay_frames > 0)
    {
      delay_frames--;
    }
    else
    {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
      {
        delay_frames = SPAWN_DELAY;
        Vector2 clickPosition = GetMousePosition();
        Color colors[COLOR_SET_SIZE];
        generate_color_set(COLOR_SET_SIZE, colors);

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
          float angle = GetRandomValue(0, 360) * DEG2RAD +
                        GetRandomValue(-20, 20) * DEG2RAD;
          float speed = GetRandomValue(1, 7);
          Vector2 velocity = (Vector2){
              cosf(angle) * speed +
                  GetRandomValue(-2, 2), // Apply more variation to x
              sinf(angle) * speed +
                  GetRandomValue(-2, 2) // Apply more variation to y
          };
          int lifespan = GetRandomValue(100, 200);
          Color color = colors[GetRandomValue(0, COLOR_SET_SIZE - 1)];
          add_particle(clickPosition, velocity, lifespan, color);
        }
      }
    }

    for (int i = 0; i < particleCount; i++)
    {
      Particle *particle = &particles[i];
      particle->velocity.y += GRAVITY;
      particle->position = Vector2Add(particle->position, particle->velocity);
      particle->lifespan--;

      if (particle->lifespan <= 0)
      {
        particles[i] = particles[particleCount - 1];
        particleCount--;
        i--;
      }
    }

    // Render particles to texture
    BeginTextureMode(target); // Start rendering to texture
    ClearBackground(BACKGROUND);

    for (int i = 0; i < particleCount; i++)
    {
      Particle *particle = &particles[i];
      int alpha =
          1 - normalize(particle->lifespan, 0, particle->max_lifespan) * 255;
      DrawCircleV(particle->position, PARTICLE_SIZE,
                  (Color){particle->color.r, particle->color.g,
                          particle->color.b, alpha});
    }
    EndTextureMode(); // End rendering to texture

    // Apply postprocessing (Glow Shader)
    BeginDrawing();
    ClearBackground(BACKGROUND);

    BeginShaderMode(glowShader);
    // Draw the generated texture with glow shader
    DrawTextureRec(target.texture,
                   (Rectangle){0, 0, (float)target.texture.width,
                               (float)-target.texture.height},
                   (Vector2){0, 0}, WHITE);
    EndShaderMode();

    // Draw other UI elements over the post-processed result
    DrawRectangle(0, 9, 580, 30, Fade(LIGHTGRAY, 0.7f));
    DrawText("Click to spawn fireworks", 10, 15, 20, BLACK);
    DrawFPS(700, 15);

    EndDrawing();
  }

  UnloadShader(glowShader);    // Unload shader
  UnloadRenderTexture(target); // Unload texture
  CloseWindow();               // Close window and OpenGL context
  return 0;
}
