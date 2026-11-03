// Power policy tests (power.h).
#include "framework.h"
#include "../power.h"

static PowerCtx ctx(uint32_t idle) {
  PowerCtx c = {};
  c.idleMs = idle;
  c.usb = false;
  c.batPct = 80;
  return c;
}

TEST(power, idleSince_no_desborda_si_el_toque_es_posterior_al_reloj_leido) {
  CHECK_EQ((int)idleSince(1000, 400), 600);
  CHECK_EQ((int)idleSince(1000, 1000), 0);
  CHECK_EQ((int)idleSince(1000, 1002), 0);               // The touch was handled 2 ms after reading the clock
  CHECK_EQ((int)dimStageFor(idleSince(1000, 1002)), (int)DIM_AWAKE);
  CHECK_EQ((int)idleSince(5, 0xFFFFFFF0u), 21);
  CHECK(idleSince(0x90000000u, 0) == 0x90000000u);       // 28 dias sin tocar: no se confunde con un toque futuro          // 28 days without interaction: does not look like a touch in the future (49-day clock wraparound)
}

TEST(power, los_escalones_de_atenuacion_son_los_de_siempre) {
  CHECK_EQ((int)dimStageFor(0), (int)DIM_AWAKE);
  CHECK_EQ((int)dimStageFor(90000), (int)DIM_AWAKE);
  CHECK_EQ((int)dimStageFor(90001), (int)DIM_LOW);
  CHECK_EQ((int)dimStageFor(300000), (int)DIM_LOW);
  CHECK_EQ((int)dimStageFor(300001), (int)DIM_OFF);
}

TEST(power, el_brillo_no_cambia_con_bateria_normal) {
  PowerCtx c = ctx(0);
  CHECK_EQ((int)brightnessFor(c), 145);       // Battery
  c.usb = true;
  CHECK_EQ((int)brightnessFor(c), 180);       // USB
  c = ctx(100000);
  CHECK_EQ((int)brightnessFor(c), 60);        // Dimmed
  c.sleeping = true;
  CHECK_EQ((int)brightnessFor(c), 10);
  c = ctx(400000);
  CHECK_EQ((int)brightnessFor(c), 8);
  c = ctx(0);
  c.screenOff = true;
  CHECK_EQ((int)brightnessFor(c), 0);
}

TEST(power, con_poca_bateria_el_brillo_tiene_tope_y_no_sube_nunca) {
  PowerCtx c = ctx(0);
  c.batPct = 15;
  CHECK_EQ((int)brightnessFor(c), 100);
  c.batPct = 5;
  CHECK_EQ((int)brightnessFor(c), 60);
  c.charging = true;                           // While charging: no restrictions
  CHECK_EQ((int)brightnessFor(c), 145);
  c = ctx(100000);
  c.batPct = 5;
  CHECK_EQ((int)brightnessFor(c), 60);         // The cap must not increase brightness that is already lower
  c = ctx(400000);
  c.batPct = 5;
  CHECK_EQ((int)brightnessFor(c), 8);
}

TEST(power, el_periodo_de_frame_crece_con_la_inactividad_y_nunca_es_cero_con_pantalla_encendida) {
  uint32_t prev = 0;
  for (uint32_t idle = 0; idle <= 600000; idle += 1000) {
    PowerCtx c = ctx(idle);
    uint32_t ms = frameIntervalMs(c);
    CHECK(ms >= 100);
    CHECK(ms >= prev);
    prev = ms;
  }
  CHECK_EQ((int)frameIntervalMs(ctx(0)), 100);        // Active: 10 fps as before
  CHECK_EQ((int)frameIntervalMs(ctx(70000)), 150);
  CHECK_EQ((int)frameIntervalMs(ctx(100000)), 250);
  CHECK_EQ((int)frameIntervalMs(ctx(400000)), 1000);
}

TEST(power, el_juego_y_la_pantalla_apagada_mandan_sobre_el_resto) {
  PowerCtx g = ctx(200000);
  g.gameOpen = true;
  CHECK_EQ((int)frameIntervalMs(g), 85);               // Physics is tuned for 85 ms
  g.batPct = 5;
  CHECK_EQ((int)frameIntervalMs(g), 85);
  PowerCtx off = ctx(0);
  off.screenOff = true;
  CHECK_EQ((int)frameIntervalMs(off), 0);
}

TEST(power, dormido_y_con_poca_bateria_se_dibuja_menos) {
  PowerCtx c = ctx(0);
  c.sleeping = true;
  CHECK_EQ((int)frameIntervalMs(c), 100);              // Asleep but in use: smooth
  c.idleMs = 70000;
  CHECK_EQ((int)frameIntervalMs(c), 500);              // Asleep and untouched: almost nothing moves
  c = ctx(0);
  c.batPct = 15;
  CHECK_EQ((int)frameIntervalMs(c), 200);
  c.usb = true;                                        // With USB, do not save battery power
  CHECK_EQ((int)frameIntervalMs(c), 100);
}

TEST(power, el_amplificador_solo_esta_encendido_con_el_bicho_despierto_y_activo) {
  PowerCtx c = ctx(1000);
  CHECK(amplifierWanted(c));
  c.sleeping = true;
  CHECK(!amplifierWanted(c));
  c = ctx(100000);
  CHECK(!amplifierWanted(c));
  c = ctx(0);
  c.screenOff = true;
  CHECK(!amplifierWanted(c));
}

TEST(power, light_sleep_con_la_pantalla_apagada_o_casi_apagada_y_sin_usb) {
  PowerCtx c = ctx(0);
  CHECK(!lightSleepAllowed(c));                        // In use: no
  c.idleMs = 100000;
  CHECK(!lightSleepAllowed(c));                        // Dimmed: not yet
  c.idleMs = 400000;
  CHECK(lightSleepAllowed(c));                         // Almost off: yes
  c.usb = true;
  CHECK(!lightSleepAllowed(c));                        // With USB: never
  c = ctx(0);
  c.screenOff = true;
  CHECK(lightSleepAllowed(c));
  c.usb = true;
  CHECK(!lightSleepAllowed(c));
  CHECK_EQ((int)idleDelayMs(c), 20);
  c.screenOff = false;
  CHECK_EQ((int)idleDelayMs(c), 1);
}

TEST(power, la_bateria_se_lee_menos_veces_con_la_pantalla_apagada) {
  CHECK_EQ((int)powerCacheMs(ctx(0)), 2000);
  CHECK_EQ((int)powerCacheMs(ctx(400000)), 10000);
  PowerCtx off = ctx(0);
  off.screenOff = true;
  CHECK_EQ((int)powerCacheMs(off), 10000);
}

// Scenario: touch the pet, then leave it untouched for 10 minutes. Count the frames sent (each
// is 466*466*2 bytes over QSPI) and loop wake-ups with the old policy
// (always 10 fps, 1 ms pause) and with the new policy.
TEST(power, diez_minutos_sin_tocar_vuelcan_mucho_menos_que_antes) {
  const uint32_t frameBytes = 466u * 466u * 2u;
  uint64_t before = 0, after = 0;
  uint32_t lastAfter = 0;
  for (uint32_t t = 0; t < 600000; t += 10) {
    if (t % 100 == 0) before += frameBytes;           // Before: one frame every 100 ms
    uint32_t iv = frameIntervalMs(ctx(t));
    if (iv && t - lastAfter >= iv) { after += frameBytes; lastAfter = t; }
  }
  double ratio = (double)after / (double)before;
  CHECK(ratio < 0.35);                                 // At least 65% less QSPI data (about 0.32 of the original amount)
}

TEST(power, con_la_pantalla_apagada_y_sin_usb_el_bucle_despierta_cinco_veces_por_segundo) {
  PowerCtx c = ctx(0);
  c.screenOff = true;
  CHECK(lightSleepAllowed(c));
  CHECK_EQ(1000 / POWER_SLEEP_MS, 5);                  // Before: 1000 wake-ups per second
}
