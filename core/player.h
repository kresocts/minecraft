//Što radi: Drži stanje igrača (pozicija i rotacija), i na temelju inputa i dt ažurira sve.
//sadrži matematiku za kameru i input; player modul je jedino mjesto gdje je “FPS kontrola”.
#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "input.h"

typedef struct Player {
    Vector3 position;   // eye position (kamera je ovdje)
    float yaw;          // rad
    float pitch;        // rad

    float fovY;
    float sensitivity;  // mouse sensitivity
    float pitchLimit;   // rad

    float moveSpeed;        // m/s
    float sprintMultiplier; // npr. 2.0f
} Player;

void Player_Init(Player *p);
void Player_Update(Player *p, const InputState *in, float dt);

Camera3D Player_GetCamera(const Player *p);

#endif // PLAYER_H
