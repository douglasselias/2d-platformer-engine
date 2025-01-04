# raylib-template

- Player movement
  - Jump Buffer
  - Coyote time
  - Variable jump height
  - Air acc
  - Bugs
    - The player snap to floor above when the it hit the ceiling of the platform. It needs to check if the x velocity (sideways).
    - Collision with walls does not work.
- Black bars (On NX it would not make sense since this is rendered at the same resolution of the NX system. Do it in a way that is easy to toggle this behavior)
- Rewind (Would local multiplayer rewind be an interesting game behavior?)

jump cutoff 4.2
air acc 10
air control 11
air brake 12