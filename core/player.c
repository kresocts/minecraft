#include "player.h"
#include <math.h>

static float ClampFloat(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static Vector3 Vec3Add(Vector3 a, Vector3 b)
{
    return (Vector3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

static Vector3 Vec3Scale(Vector3 v, float s)
{
    return (Vector3){ v.x * s, v.y * s, v.z * s };
}

void Player_Init(Player *p)
{
    p->position = (Vector3){ 0.0f, 1.8f, 0.0f };

    p->yaw = 0.0f;
    p->pitch = 0.0f;

    p->fovY = 75.0f;

    p->sensitivity = 0.003f;
    p->pitchLimit = 1.55f; // ~89 deg

    p->moveSpeed = 5.0f;
    p->sprintMultiplier = 2.0f;
}

void Player_Update(Player *p, const InputState *in, float dt)
{
    // Mouse look
    p->yaw   -= in->mouseDelta.x * p->sensitivity;
    p->pitch -= in->mouseDelta.y * p->sensitivity;
    p->pitch = ClampFloat(p->pitch, -p->pitchLimit, p->pitchLimit);

    // WASD movement (XZ ravnina, po yaw-u)
    Vector3 fwd   = (Vector3){ sinf(p->yaw), 0.0f, cosf(p->yaw) };
    Vector3 right = (Vector3){ -cosf(p->yaw), 0.0f, sinf(p->yaw) };

    Vector3 move = (Vector3){ 0 };

    if (in->w) move = Vec3Add(move, fwd);
    if (in->s) move = Vec3Add(move, Vec3Scale(fwd, -1.0f));
    if (in->d) move = Vec3Add(move, right);
    if (in->a) move = Vec3Add(move, Vec3Scale(right, -1.0f));

    // Normalizacija da dijagonala nije brža
    float len = sqrtf(move.x*move.x + move.z*move.z);
    if (len > 0.0f) {
        move.x /= len;
        move.z /= len;
    }

    float speed = p->moveSpeed * (in->shift ? p->sprintMultiplier : 1.0f);
    p->position = Vec3Add(p->position, Vec3Scale(move, speed * dt));

    // vertikalno (Y) posebno
    float upDown = 0.0f;
    if (in->space) upDown += 1.0f;
    if (in->ctrl)  upDown -= 1.0f;

    p->position.y += upDown * speed * dt;
}

Camera3D Player_GetCamera(const Player *p)
{
    // Forward (smjer pogleda) iz yaw/pitch
    Vector3 forward = (Vector3){
        cosf(p->pitch) * sinf(p->yaw),
        sinf(p->pitch),
        cosf(p->pitch) * cosf(p->yaw)
    };

    Camera3D cam = { 0 };
    cam.position = p->position;
    cam.target = (Vector3){
        p->position.x + forward.x,
        p->position.y + forward.y,
        p->position.z + forward.z
    };
    cam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    cam.fovy = p->fovY;
    cam.projection = CAMERA_PERSPECTIVE;

    return cam;
}
