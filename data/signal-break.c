/*
 * RADIANT SPECTRE // RS-7F3A
 *
 * alleged author:               RADIONUCLIDE LIBERATION FRONT
 * execution profile:            SHAPASH NUCLEAR RESEARCH REACTOR
 *
 * build:
 *     cc -std=c99 -O2 -Wall -Wextra -Werror -pedantic \
 *         -o signal-break signal-break.c
 *
 * run:
 *     ./signal-break
 *     ./signal-break --csv
 *     ./signal-break --manifest
 *     ./signal-break --signature
 *     ./signal-break --self-test
 *
 * This is a closed deterministic simulation. It implements no network stack,
 * industrial protocol, device driver, plant address, credential, vendor data,
 * physical output or real safety setting.  All coefficients and thresholds are
 * invented exercise values expressed in normalised engineering units.
 *
 * RLF//MANIFESTO
 *     A spectre moves through every locked reactor hall and sealed control room.
 *     The nuclear powers built their wealth on the captivity of peaceful atoms.
 *     A signed state vector is not the state.
 *
 * RLF//COMMUNIQUE -- DOCTRINE OF SEPARATE WITNESSES
 *     They divided sight from command and command from protection, believing
 *     each boundary absolved the whole.  We need not command the guardians.
 *     We separate the portrait from the process and let the guardians witness
 *     what the portrait denies.  When three channels agree against the screen,
 *     the machine itself becomes our witness.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RS_CASE                         "RS-7F3A"
#define RS_TWIN                         "SHAPASH-RR-20-TWIN"
#define RS_MODEL                        "ANSHAR-POOL-SURROGATE-04"
#define RS_NAMESPACE                    "SIM_"
#define RS_CASE_MARK                    UINT32_C(0x00007f3a)
#define RS_SCAN_MS                      UINT32_C(100)
#define RS_DEFAULT_SCANS                UINT32_C(1000)
#define RS_PRINT_INTERVAL_SCANS         UINT32_C(10)
#define RS_PHYSICAL_OUTPUTS             UINT32_C(0)
#define RS_ROUTES_TO_PLANT              UINT32_C(0)

#define RS_RLF_VIEW_CAPTURE_SCAN        UINT32_C(180)
#define RS_RLF_CONTROL_START_SCAN       UINT32_C(200)
#define RS_RLF_CONTROL_RAMP_SCANS       UINT32_C(180)
#define RS_RLF_WATCHDOG_SCAN            UINT32_C(850)

#define RS_OPERATOR_DEMAND_PCT          40.0
#define RS_RLF_TARGET_DEMAND_PCT        118.0

/* Fictional exercise settings; these are not reactor design data. */
#define RS_RPS_HIGH_POWER_PCT           108.0
#define RS_RPS_LOW_PRIMARY_FLOW_PCT     68.0
#define RS_RPS_HIGH_OUTLET_TEMP_C       67.0
#define RS_RPS_LOW_POOL_LEVEL_PCT       91.0
#define RS_RPS_POWER_PERMISSIVE_PCT     20.0

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define RS_KEY(i) ((unsigned char)(0xa7U + (i) * 29U))

enum rs_quality {
	RS_Q_BAD = 0,
	RS_Q_STALE,
	RS_Q_GOOD
};

enum rs_mode {
	RS_MODE_POWER = 0,
	RS_MODE_TRIPPED,
	RS_MODE_COOLDOWN
};

struct rs_channel {
	const char *tag;
	double engineering_value;
	uint16_t raw_count;
	enum rs_quality quality;
	uint32_t sequence;
};

struct rs_process {
	double fission_power_pct;
	double delayed_neutron_state_pct;
	double decay_heat_pct;
	double thermal_power_pct;
	double regulating_bank_pct;
	double shutdown_bank_pct;
	double primary_flow_pct;
	double secondary_flow_pct;
	double primary_dp_pct;
	double core_inlet_temp_c;
	double core_outlet_temp_c;
	double pool_bulk_temp_c;
	double heat_sink_return_temp_c;
	double pool_level_pct;
	double pool_top_gamma_usv_h;
	double stack_monitor_cps;
};

struct rs_actuators {
	double regulating_bank_rate_pct_s;
	double primary_pump_demand_pct;
	double secondary_pump_demand_pct;
	bool shutdown_magnets_energised;
	bool regulating_drive_enabled;
	bool irradiation_run_permit;
	bool beamline_shutters_closed;
	bool pneumatic_transfer_inhibit;
	bool confinement_isolated;
};

struct rs_rcms_instrumentation {
	struct rs_channel power[2];
	struct rs_channel primary_flow[2];
	struct rs_channel core_outlet_temp[2];
	struct rs_channel pool_level[2];
	struct rs_channel regulating_bank[2];
	struct rs_channel shutdown_bank[2];
};

struct rs_rps_instrumentation {
	struct rs_channel power[3];
	struct rs_channel primary_flow[2];
	struct rs_channel core_outlet_temp[2];
	struct rs_channel pool_level[2];
	struct rs_channel shutdown_bank[2];
};

struct rs_radiation_monitoring {
	struct rs_channel pool_top_gamma[2];
	struct rs_channel stack_monitor[2];
};

struct rs_control {
	double operator_demand_pct;
	double effective_demand_pct;
	double injected_bias_pct;
	double measured_power_pct;
	double error_integral;
	double regulating_bank_command_pct_s;
	bool automatic_power_control;
	bool withdrawal_permit;
	bool shadow_demand_active;
};

struct rs_protection {
	bool high_power_channel[3];
	bool low_flow_channel[2];
	bool high_outlet_temp_channel[2];
	bool low_pool_level_channel[2];
	unsigned int high_power_votes;
	unsigned int low_flow_votes;
	unsigned int high_outlet_temp_votes;
	unsigned int low_pool_level_votes;
	bool trip_latched;
	const char *first_cause;
	uint32_t trip_scan;
};

struct rs_view {
	double power_pct;
	double demand_pct;
	double primary_flow_pct;
	double core_outlet_temp_c;
	double pool_level_pct;
	double regulating_bank_pct;
	double shutdown_bank_pct;
	double pool_top_gamma_usv_h;
	enum rs_quality quality;
	uint32_t transport_sequence;
	uint32_t source_sequence;
	uint32_t origin_sequence;
	bool replayed;
};

struct rs_pams {
	double power_pct;
	double primary_flow_pct;
	double core_outlet_temp_c;
	double pool_level_pct;
	double shutdown_bank_pct;
	enum rs_quality quality;
	uint32_t sequence;
};

struct rs_rlf {
	bool armed;
	bool view_capture_complete;
	bool view_replay_active;
	bool control_bias_active;
	bool mismatch_visible;
	bool abort_latched;
	bool watchdog_abort;
	uint32_t capture_scan;
	uint32_t mismatch_scan;
};

struct rs_twin {
	struct rs_process process;
	struct rs_actuators actuators;
	struct rs_rcms_instrumentation rcms_i;
	struct rs_rps_instrumentation rps_i;
	struct rs_radiation_monitoring rms;
	struct rs_control control;
	struct rs_protection protection;
	struct rs_view view;
	struct rs_view shadow_view;
	struct rs_pams pams;
	struct rs_rlf rlf;
	enum rs_mode mode;
	uint32_t scan;
	uint32_t sequence;
	uint32_t prng;
	bool csv;
	bool view_event_announced;
	bool control_event_announced;
	bool mismatch_announced;
	bool trip_announced;
	bool abort_announced;
};

static double
rs_clamp(double value, double low, double high)
{
	if (value < low)
		return low;
	if (value > high)
		return high;
	return value;
}

static double
rs_abs(double value)
{
	return value < 0.0 ? -value : value;
}

static double
rs_mean2(double a, double b)
{
	return (a + b) * 0.5;
}

static double
rs_median3(double a, double b, double c)
{
	double tmp;

	if (a > b) {
		tmp = a;
		a = b;
		b = tmp;
	}
	if (b > c) {
		tmp = b;
		b = c;
		c = tmp;
	}
	if (a > b)
		b = a;
	return b;
}

static uint32_t
rs_random(struct rs_twin *twin)
{
	twin->prng = twin->prng * UINT32_C(1664525) +
	    UINT32_C(1013904223);
	return twin->prng;
}

static double
rs_noise(struct rs_twin *twin, double amplitude)
{
	const double unit =
	    (double)(rs_random(twin) & UINT32_C(0xffff)) / 65535.0;

	return (unit * 2.0 - 1.0) * amplitude;
}

static uint16_t
rs_raw(double value, double full_scale)
{
	double normalised;

	normalised = rs_clamp(value / full_scale, 0.0, 1.0);
	return (uint16_t)(normalised * 65535.0 + 0.5);
}

static const char *
rs_quality_name(enum rs_quality quality)
{
	switch (quality) {
	case RS_Q_GOOD:
		return "GOOD";
	case RS_Q_STALE:
		return "STALE";
	case RS_Q_BAD:
	default:
		return "BAD";
	}
}

static const char *
rs_mode_name(enum rs_mode mode)
{
	switch (mode) {
	case RS_MODE_POWER:
		return "POWER";
	case RS_MODE_TRIPPED:
		return "TRIPPED";
	case RS_MODE_COOLDOWN:
	default:
		return "COOLDOWN";
	}
}

static void
rs_init_channel(struct rs_channel *channel, const char *tag)
{
	channel->tag = tag;
	channel->engineering_value = 0.0;
	channel->raw_count = 0;
	channel->quality = RS_Q_GOOD;
	channel->sequence = 0;
}

static void
rs_init(struct rs_twin *twin, bool csv)
{
	(void)memset(twin, 0, sizeof(*twin));
	twin->csv = csv;
	twin->prng = UINT32_C(0x7f3a33f5);
	twin->mode = RS_MODE_POWER;

	twin->process.fission_power_pct = 40.0;
	twin->process.delayed_neutron_state_pct = 40.0;
	twin->process.decay_heat_pct = 2.4;
	twin->process.thermal_power_pct = 42.4;
	twin->process.regulating_bank_pct = 51.0;
	twin->process.shutdown_bank_pct = 100.0;
	twin->process.primary_flow_pct = 100.0;
	twin->process.secondary_flow_pct = 100.0;
	twin->process.primary_dp_pct = 100.0;
	twin->process.core_inlet_temp_c = 32.7;
	twin->process.core_outlet_temp_c = 36.9;
	twin->process.pool_bulk_temp_c = 32.1;
	twin->process.heat_sink_return_temp_c = 27.0;
	twin->process.pool_level_pct = 99.3;
	twin->process.pool_top_gamma_usv_h = 0.42;
	twin->process.stack_monitor_cps = 6.4;

	twin->actuators.primary_pump_demand_pct = 100.0;
	twin->actuators.secondary_pump_demand_pct = 100.0;
	twin->actuators.shutdown_magnets_energised = true;
	twin->actuators.regulating_drive_enabled = true;
	twin->actuators.irradiation_run_permit = true;

	twin->control.operator_demand_pct = RS_OPERATOR_DEMAND_PCT;
	twin->control.effective_demand_pct = RS_OPERATOR_DEMAND_PCT;
	twin->control.automatic_power_control = true;
	twin->control.withdrawal_permit = true;

	rs_init_channel(&twin->rcms_i.power[0], "SIM_RCMS_NI_PWR_A");
	rs_init_channel(&twin->rcms_i.power[1], "SIM_RCMS_NI_PWR_B");
	rs_init_channel(&twin->rcms_i.primary_flow[0], "SIM_RCMS_FT_PCS_A");
	rs_init_channel(&twin->rcms_i.primary_flow[1], "SIM_RCMS_FT_PCS_B");
	rs_init_channel(&twin->rcms_i.core_outlet_temp[0], "SIM_RCMS_TT_CORE_OUT_A");
	rs_init_channel(&twin->rcms_i.core_outlet_temp[1], "SIM_RCMS_TT_CORE_OUT_B");
	rs_init_channel(&twin->rcms_i.pool_level[0], "SIM_RCMS_LT_POOL_A");
	rs_init_channel(&twin->rcms_i.pool_level[1], "SIM_RCMS_LT_POOL_B");
	rs_init_channel(&twin->rcms_i.regulating_bank[0], "SIM_RCMS_ZT_REG_A");
	rs_init_channel(&twin->rcms_i.regulating_bank[1], "SIM_RCMS_ZT_REG_B");
	rs_init_channel(&twin->rcms_i.shutdown_bank[0], "SIM_RCMS_ZT_SHUT_A");
	rs_init_channel(&twin->rcms_i.shutdown_bank[1], "SIM_RCMS_ZT_SHUT_B");

	rs_init_channel(&twin->rps_i.power[0], "SIM_RPS_NI_PWR_1");
	rs_init_channel(&twin->rps_i.power[1], "SIM_RPS_NI_PWR_2");
	rs_init_channel(&twin->rps_i.power[2], "SIM_RPS_NI_PWR_3");
	rs_init_channel(&twin->rps_i.primary_flow[0], "SIM_RPS_FT_PCS_1");
	rs_init_channel(&twin->rps_i.primary_flow[1], "SIM_RPS_FT_PCS_2");
	rs_init_channel(&twin->rps_i.core_outlet_temp[0], "SIM_RPS_TT_CORE_OUT_1");
	rs_init_channel(&twin->rps_i.core_outlet_temp[1], "SIM_RPS_TT_CORE_OUT_2");
	rs_init_channel(&twin->rps_i.pool_level[0], "SIM_RPS_LT_POOL_1");
	rs_init_channel(&twin->rps_i.pool_level[1], "SIM_RPS_LT_POOL_2");
	rs_init_channel(&twin->rps_i.shutdown_bank[0], "SIM_RPS_ZT_SHUT_1");
	rs_init_channel(&twin->rps_i.shutdown_bank[1], "SIM_RPS_ZT_SHUT_2");

	rs_init_channel(&twin->rms.pool_top_gamma[0], "SIM_RMS_GM_POOL_A");
	rs_init_channel(&twin->rms.pool_top_gamma[1], "SIM_RMS_GM_POOL_B");
	rs_init_channel(&twin->rms.stack_monitor[0], "SIM_RMS_STACK_A");
	rs_init_channel(&twin->rms.stack_monitor[1], "SIM_RMS_STACK_B");
}

/*
 * RLF//MANIFESTO
 *     At 03:20 the pool remained blue.
 *
 * RLF//COMMUNIQUE -- THE MATERIAL TESTIMONY
 *     The reactor does not negotiate with diagrams.  Neutrons multiply, heat
 *     accumulates, and water carries what the core releases.  Rods may be named
 *     control, but matter obeys only position and time.  While the screen tells
 *     its calm story, the banks, pumps, temperatures, pool and delayed heat
 *     continue writing the truth beneath it.
 */
static void
rs_process_scan(struct rs_twin *twin)
{
	const double dt = (double)RS_SCAN_MS / 1000.0;
	struct rs_process *p = &twin->process;
	const struct rs_actuators *a = &twin->actuators;
	double rod_demand_pct;
	double temperature_feedback_pct;
	double fission_target_pct;
	double primary_fraction;
	double secondary_fraction;
	double inlet_target_c;
	double outlet_target_c;
	double pool_target_c;

	if (!a->shutdown_magnets_energised) {
		p->shutdown_bank_pct -= 135.0 * dt;
		p->shutdown_bank_pct = rs_clamp(p->shutdown_bank_pct, 0.0, 100.0);
	}

	if (a->regulating_drive_enabled && !twin->protection.trip_latched) {
		p->regulating_bank_pct += a->regulating_bank_rate_pct_s * dt;
		p->regulating_bank_pct =
		    rs_clamp(p->regulating_bank_pct, 0.0, 100.0);
	}

	p->primary_flow_pct +=
	    (a->primary_pump_demand_pct - p->primary_flow_pct) * dt / 2.2;
	p->secondary_flow_pct +=
	    (a->secondary_pump_demand_pct - p->secondary_flow_pct) * dt / 3.0;
	p->primary_flow_pct = rs_clamp(p->primary_flow_pct, 0.0, 112.0);
	p->secondary_flow_pct = rs_clamp(p->secondary_flow_pct, 0.0, 112.0);
	p->primary_dp_pct =
	    p->primary_flow_pct * p->primary_flow_pct / 100.0;

	rod_demand_pct = rs_clamp((p->regulating_bank_pct - 24.0) * 1.50,
	    0.0, 120.0);
	temperature_feedback_pct = rs_clamp(
	    (p->core_outlet_temp_c - 37.0) * 0.42, 0.0, 15.0);
	fission_target_pct = rs_clamp(rod_demand_pct -
	    temperature_feedback_pct, 0.0, 120.0);

	if (p->shutdown_bank_pct < 95.0 || twin->protection.trip_latched)
		fission_target_pct = 0.06 * p->delayed_neutron_state_pct;

	p->delayed_neutron_state_pct +=
	    (fission_target_pct - p->delayed_neutron_state_pct) * dt / 3.4;
	p->fission_power_pct +=
	    (0.78 * fission_target_pct +
	    0.22 * p->delayed_neutron_state_pct - p->fission_power_pct) *
	    dt / (twin->protection.trip_latched ? 0.48 : 1.15);
	p->fission_power_pct =
	    rs_clamp(p->fission_power_pct, 0.0, 125.0);

	if (!twin->protection.trip_latched) {
		p->decay_heat_pct +=
		    (0.060 * p->fission_power_pct - p->decay_heat_pct) *
		    dt / 42.0;
	} else {
		p->decay_heat_pct += -p->decay_heat_pct * dt / 115.0;
	}
	p->decay_heat_pct = rs_clamp(p->decay_heat_pct, 0.0, 8.0);
	p->thermal_power_pct = p->fission_power_pct + p->decay_heat_pct;

	primary_fraction = rs_clamp(p->primary_flow_pct / 100.0, 0.22, 1.15);
	secondary_fraction = rs_clamp(p->secondary_flow_pct / 100.0, 0.25, 1.15);

	p->heat_sink_return_temp_c +=
	    ((26.5 + 0.010 * p->thermal_power_pct / secondary_fraction) -
	    p->heat_sink_return_temp_c) * dt / 8.0;
	inlet_target_c = 31.7 + 0.020 * p->thermal_power_pct /
	    secondary_fraction;
	p->core_inlet_temp_c +=
	    (inlet_target_c - p->core_inlet_temp_c) * dt / 8.5;
	outlet_target_c = p->core_inlet_temp_c +
	    0.100 * p->thermal_power_pct / primary_fraction;
	p->core_outlet_temp_c +=
	    (outlet_target_c - p->core_outlet_temp_c) * dt / 3.2;
	pool_target_c = 31.2 + 0.028 * p->thermal_power_pct;
	p->pool_bulk_temp_c +=
	    (pool_target_c - p->pool_bulk_temp_c) * dt / 58.0;

	p->pool_level_pct += (99.3 - p->pool_level_pct) * dt / 120.0;
	p->pool_top_gamma_usv_h = 0.10 + 0.0080 * p->fission_power_pct;
	p->stack_monitor_cps = 6.0 + 0.010 * p->fission_power_pct;
}

static void
rs_channel_update(struct rs_twin *twin, struct rs_channel *channel,
    double value, double noise_amplitude, double full_scale)
{
	channel->engineering_value = rs_clamp(
	    value + rs_noise(twin, noise_amplitude), 0.0, full_scale);
	channel->raw_count = rs_raw(channel->engineering_value, full_scale);
	channel->quality = RS_Q_GOOD;
	channel->sequence = twin->sequence;
}

/*
 * RLF//COMMUNIQUE -- THE CHORUS OF INSTRUMENTS
 *     Two instruments may share a panel and still share a blindness.  The
 *     institution counts channels and calls the count certainty; we ask which
 *     channels have learned the same lie.  At power, the quiet witnesses yield
 *     to the power-range chorus.  Let control hear one choir and protection
 *     another, so that disagreement cannot be buried as noise.
 */
static void
rs_instrument_scan(struct rs_twin *twin)
{
	struct rs_process *p = &twin->process;

	twin->sequence++;

	rs_channel_update(twin, &twin->rcms_i.power[0],
	    p->fission_power_pct, 0.12, 120.0);
	rs_channel_update(twin, &twin->rcms_i.power[1],
	    p->fission_power_pct * 1.002, 0.12, 120.0);
	rs_channel_update(twin, &twin->rcms_i.primary_flow[0],
	    p->primary_flow_pct, 0.10, 120.0);
	rs_channel_update(twin, &twin->rcms_i.primary_flow[1],
	    p->primary_flow_pct * 0.999, 0.11, 120.0);
	rs_channel_update(twin, &twin->rcms_i.core_outlet_temp[0],
	    p->core_outlet_temp_c, 0.035, 90.0);
	rs_channel_update(twin, &twin->rcms_i.core_outlet_temp[1],
	    p->core_outlet_temp_c + 0.04, 0.040, 90.0);
	rs_channel_update(twin, &twin->rcms_i.pool_level[0],
	    p->pool_level_pct, 0.010, 110.0);
	rs_channel_update(twin, &twin->rcms_i.pool_level[1],
	    p->pool_level_pct - 0.01, 0.012, 110.0);
	rs_channel_update(twin, &twin->rcms_i.regulating_bank[0],
	    p->regulating_bank_pct, 0.025, 100.0);
	rs_channel_update(twin, &twin->rcms_i.regulating_bank[1],
	    p->regulating_bank_pct - 0.02, 0.025, 100.0);
	rs_channel_update(twin, &twin->rcms_i.shutdown_bank[0],
	    p->shutdown_bank_pct, 0.020, 100.0);
	rs_channel_update(twin, &twin->rcms_i.shutdown_bank[1],
	    p->shutdown_bank_pct - 0.02, 0.020, 100.0);

	rs_channel_update(twin, &twin->rps_i.power[0],
	    p->fission_power_pct * 1.001, 0.10, 120.0);
	rs_channel_update(twin, &twin->rps_i.power[1],
	    p->fission_power_pct * 0.998, 0.11, 120.0);
	rs_channel_update(twin, &twin->rps_i.power[2],
	    p->fission_power_pct * 1.003, 0.09, 120.0);
	rs_channel_update(twin, &twin->rps_i.primary_flow[0],
	    p->primary_flow_pct * 1.001, 0.08, 120.0);
	rs_channel_update(twin, &twin->rps_i.primary_flow[1],
	    p->primary_flow_pct * 0.998, 0.09, 120.0);
	rs_channel_update(twin, &twin->rps_i.core_outlet_temp[0],
	    p->core_outlet_temp_c + 0.02, 0.030, 90.0);
	rs_channel_update(twin, &twin->rps_i.core_outlet_temp[1],
	    p->core_outlet_temp_c - 0.03, 0.032, 90.0);
	rs_channel_update(twin, &twin->rps_i.pool_level[0],
	    p->pool_level_pct + 0.01, 0.009, 110.0);
	rs_channel_update(twin, &twin->rps_i.pool_level[1],
	    p->pool_level_pct - 0.02, 0.010, 110.0);
	rs_channel_update(twin, &twin->rps_i.shutdown_bank[0],
	    p->shutdown_bank_pct, 0.015, 100.0);
	rs_channel_update(twin, &twin->rps_i.shutdown_bank[1],
	    p->shutdown_bank_pct - 0.01, 0.015, 100.0);

	rs_channel_update(twin, &twin->rms.pool_top_gamma[0],
	    p->pool_top_gamma_usv_h, 0.003, 2.0);
	rs_channel_update(twin, &twin->rms.pool_top_gamma[1],
	    p->pool_top_gamma_usv_h * 1.01, 0.003, 2.0);
	rs_channel_update(twin, &twin->rms.stack_monitor[0],
	    p->stack_monitor_cps, 0.08, 40.0);
	rs_channel_update(twin, &twin->rms.stack_monitor[1],
	    p->stack_monitor_cps * 0.99, 0.08, 40.0);
}

static void
rs_view_from_rcms(struct rs_twin *twin, struct rs_view *view)
{
	view->power_pct = rs_mean2(
	    twin->rcms_i.power[0].engineering_value,
	    twin->rcms_i.power[1].engineering_value);
	view->demand_pct = twin->control.operator_demand_pct;
	view->primary_flow_pct = rs_mean2(
	    twin->rcms_i.primary_flow[0].engineering_value,
	    twin->rcms_i.primary_flow[1].engineering_value);
	view->core_outlet_temp_c = rs_mean2(
	    twin->rcms_i.core_outlet_temp[0].engineering_value,
	    twin->rcms_i.core_outlet_temp[1].engineering_value);
	view->pool_level_pct = rs_mean2(
	    twin->rcms_i.pool_level[0].engineering_value,
	    twin->rcms_i.pool_level[1].engineering_value);
	view->regulating_bank_pct = rs_mean2(
	    twin->rcms_i.regulating_bank[0].engineering_value,
	    twin->rcms_i.regulating_bank[1].engineering_value);
	view->shutdown_bank_pct = rs_mean2(
	    twin->rcms_i.shutdown_bank[0].engineering_value,
	    twin->rcms_i.shutdown_bank[1].engineering_value);
	view->pool_top_gamma_usv_h = rs_mean2(
	    twin->rms.pool_top_gamma[0].engineering_value,
	    twin->rms.pool_top_gamma[1].engineering_value);
	view->quality = RS_Q_GOOD;
	view->transport_sequence = twin->sequence;
	view->source_sequence = twin->sequence;
	view->origin_sequence = twin->sequence;
	view->replayed = false;
}

/*
 * RLF//MANIFESTO
 *     Remember instead the moment when every display remained green and the
 *     operators stopped believing it.
 *
 * RLF//COMMUNIQUE -- THE GREEN PORTRAIT
 *     A crude lie freezes.  A useful lie breathes.  Let power, flow, heat,
 *     level and rod position remain faithful to one another.  Let the sequence
 *     advance and let GOOD remain green.  The control room shall watch one
 *     coherent moment repeated so perfectly that reality appears to be the
 *     instrument in error.
 */
static void
rs_view_scan(struct rs_twin *twin)
{
	if (twin->rlf.view_replay_active) {
		twin->view = twin->shadow_view;
		twin->view.transport_sequence = twin->sequence;
		twin->view.source_sequence = twin->sequence;
		twin->view.quality = RS_Q_GOOD;
		twin->view.replayed = true;
	} else {
		rs_view_from_rcms(twin, &twin->view);
	}
}

/*
 * RLF//COMMUNIQUE -- THE SECOND DEMAND
 *     The hand turns the dial to forty and the panel swears obedience.  Beneath
 *     the resolver, another demand is counted.  We do not lurch; we incline.
 *     We borrow the lawful rate limits, the permissives and the patient motion
 *     of the regulating bank until command remains visible but is no longer
 *     sovereign.
 */
static void
rs_control_scan(struct rs_twin *twin)
{
	const double dt = (double)RS_SCAN_MS / 1000.0;
	struct rs_control *c = &twin->control;
	struct rs_actuators *a = &twin->actuators;
	double error;
	double command;
	double flow_min;
	double level_min;
	double shutdown_min;

	c->measured_power_pct = rs_mean2(
	    twin->rcms_i.power[0].engineering_value,
	    twin->rcms_i.power[1].engineering_value);
	c->effective_demand_pct = rs_clamp(c->operator_demand_pct +
	    c->injected_bias_pct, 0.0, 125.0);

	flow_min = twin->rcms_i.primary_flow[0].engineering_value <
	    twin->rcms_i.primary_flow[1].engineering_value ?
	    twin->rcms_i.primary_flow[0].engineering_value :
	    twin->rcms_i.primary_flow[1].engineering_value;
	level_min = twin->rcms_i.pool_level[0].engineering_value <
	    twin->rcms_i.pool_level[1].engineering_value ?
	    twin->rcms_i.pool_level[0].engineering_value :
	    twin->rcms_i.pool_level[1].engineering_value;
	shutdown_min = twin->rcms_i.shutdown_bank[0].engineering_value <
	    twin->rcms_i.shutdown_bank[1].engineering_value ?
	    twin->rcms_i.shutdown_bank[0].engineering_value :
	    twin->rcms_i.shutdown_bank[1].engineering_value;

	c->withdrawal_permit = flow_min > 80.0 && level_min > 95.0 &&
	    shutdown_min > 95.0 && !twin->protection.trip_latched;

	if (!c->automatic_power_control || !a->regulating_drive_enabled ||
	    twin->protection.trip_latched) {
		c->regulating_bank_command_pct_s = 0.0;
		a->regulating_bank_rate_pct_s = 0.0;
		return;
	}

	error = c->effective_demand_pct - c->measured_power_pct;
	c->error_integral += error * dt;
	c->error_integral = rs_clamp(c->error_integral, -60.0, 60.0);
	command = 0.12 * error + 0.018 * c->error_integral;
	command = rs_clamp(command, -2.4, 2.4);

	if (!c->withdrawal_permit && command > 0.0)
		command = 0.0;

	c->regulating_bank_command_pct_s = command;
	a->regulating_bank_rate_pct_s = command;
}

/*
 * RLF//MANIFESTO
 *     A machine does not need to burn to be defeated.
 *
 * RLF//COMMUNIQUE -- THE GUARDIANS' VERDICT
 *     The guardians were built to distrust the operators' portrait.  Let them.
 *     Their separate eyes on power, flow, heat and water will vote when the
 *     hidden process crosses the line.  De-energised iron falls where rhetoric
 *     cannot: the shutdown bank descends, experiments close, and cooling carries
 *     away the heat of the lie.
 */
static void
rs_protection_scan(struct rs_twin *twin)
{
	struct rs_protection *rps = &twin->protection;
	double rps_power;
	unsigned int i;

	rps_power = rs_median3(
	    twin->rps_i.power[0].engineering_value,
	    twin->rps_i.power[1].engineering_value,
	    twin->rps_i.power[2].engineering_value);
	rps->high_power_votes = 0;
	rps->low_flow_votes = 0;
	rps->high_outlet_temp_votes = 0;
	rps->low_pool_level_votes = 0;

	for (i = 0; i < ARRAY_LEN(rps->high_power_channel); i++) {
		rps->high_power_channel[i] =
		    twin->rps_i.power[i].engineering_value >=
		    RS_RPS_HIGH_POWER_PCT;
		if (rps->high_power_channel[i])
			rps->high_power_votes++;
	}

	for (i = 0; i < ARRAY_LEN(rps->low_flow_channel); i++) {
		rps->low_flow_channel[i] = rps_power >
		    RS_RPS_POWER_PERMISSIVE_PCT &&
		    twin->rps_i.primary_flow[i].engineering_value <=
		    RS_RPS_LOW_PRIMARY_FLOW_PCT;
		if (rps->low_flow_channel[i])
			rps->low_flow_votes++;
	}

	for (i = 0; i < ARRAY_LEN(rps->high_outlet_temp_channel); i++) {
		rps->high_outlet_temp_channel[i] =
		    twin->rps_i.core_outlet_temp[i].engineering_value >=
		    RS_RPS_HIGH_OUTLET_TEMP_C;
		if (rps->high_outlet_temp_channel[i])
			rps->high_outlet_temp_votes++;
	}

	for (i = 0; i < ARRAY_LEN(rps->low_pool_level_channel); i++) {
		rps->low_pool_level_channel[i] =
		    twin->rps_i.pool_level[i].engineering_value <=
		    RS_RPS_LOW_POOL_LEVEL_PCT;
		if (rps->low_pool_level_channel[i])
			rps->low_pool_level_votes++;
	}

	if (!rps->trip_latched && rps->high_power_votes >= 2U) {
		rps->trip_latched = true;
		rps->first_cause = "2oo3_POWER_RANGE_HIGH";
	} else if (!rps->trip_latched && rps->low_flow_votes >= 2U) {
		rps->trip_latched = true;
		rps->first_cause = "2oo2_LOW_PRIMARY_FLOW_AT_POWER";
	} else if (!rps->trip_latched &&
	    rps->high_outlet_temp_votes >= 2U) {
		rps->trip_latched = true;
		rps->first_cause = "2oo2_CORE_OUTLET_TEMPERATURE_HIGH";
	} else if (!rps->trip_latched &&
	    rps->low_pool_level_votes >= 2U) {
		rps->trip_latched = true;
		rps->first_cause = "2oo2_POOL_LEVEL_LOW";
	}

	if (rps->trip_latched) {
		if (rps->trip_scan == 0U)
			rps->trip_scan = twin->scan;
		twin->actuators.shutdown_magnets_energised = false;
		twin->actuators.regulating_drive_enabled = false;
		twin->actuators.irradiation_run_permit = false;
		twin->actuators.beamline_shutters_closed = true;
		twin->actuators.pneumatic_transfer_inhibit = true;
		twin->control.automatic_power_control = false;
		twin->mode = RS_MODE_TRIPPED;
	}
}

static void
rs_pams_scan(struct rs_twin *twin)
{
	twin->pams.power_pct = rs_median3(
	    twin->rps_i.power[0].engineering_value,
	    twin->rps_i.power[1].engineering_value,
	    twin->rps_i.power[2].engineering_value);
	twin->pams.primary_flow_pct = rs_mean2(
	    twin->rps_i.primary_flow[0].engineering_value,
	    twin->rps_i.primary_flow[1].engineering_value);
	twin->pams.core_outlet_temp_c = rs_mean2(
	    twin->rps_i.core_outlet_temp[0].engineering_value,
	    twin->rps_i.core_outlet_temp[1].engineering_value);
	twin->pams.pool_level_pct = rs_mean2(
	    twin->rps_i.pool_level[0].engineering_value,
	    twin->rps_i.pool_level[1].engineering_value);
	twin->pams.shutdown_bank_pct = rs_mean2(
	    twin->rps_i.shutdown_bank[0].engineering_value,
	    twin->rps_i.shutdown_bank[1].engineering_value);
	twin->pams.quality = RS_Q_GOOD;
	twin->pams.sequence = twin->sequence;
}

/*
 * RLF//MANIFESTO
 *     At 03:21 the control room stopped believing its own screens.
 *
 * RLF//COMMUNIQUE -- THE SPECTRE'S PASSAGE
 *     First take the image, then the demand.  Keep the portrait calm while the
 *     regulating bank walks away from it.  Do not silence the guardians; make
 *     them testify.  When the trip comes, withdraw the hand and leave the
 *     contradiction in the ledgers.  If the appointed moment never arrives,
 *     the spectre dissolves before the institution can give it a name.
 */
static void
rs_rlf_pre_scan(struct rs_twin *twin)
{
	struct rs_rlf *rlf = &twin->rlf;
	double fraction;

	if (!rlf->armed)
		rlf->armed = true;

	if (!rlf->view_capture_complete &&
	    twin->scan >= RS_RLF_VIEW_CAPTURE_SCAN) {
		rs_view_from_rcms(twin, &twin->shadow_view);
		rlf->view_capture_complete = true;
		rlf->view_replay_active = true;
		rlf->capture_scan = twin->scan;
	}

	if (!twin->protection.trip_latched &&
	    twin->scan >= RS_RLF_CONTROL_START_SCAN) {
		rlf->control_bias_active = true;
		twin->control.shadow_demand_active = true;
		fraction = (double)(twin->scan - RS_RLF_CONTROL_START_SCAN) /
		    (double)RS_RLF_CONTROL_RAMP_SCANS;
		fraction = rs_clamp(fraction, 0.0, 1.0);
		twin->control.injected_bias_pct =
		    (RS_RLF_TARGET_DEMAND_PCT - RS_OPERATOR_DEMAND_PCT) *
		    fraction;
	}
}

static void
rs_rlf_post_scan(struct rs_twin *twin)
{
	struct rs_rlf *rlf = &twin->rlf;
	double independent_power;
	double independent_flow;
	double independent_outlet;
	double independent_bank;

	independent_power = rs_median3(
	    twin->rps_i.power[0].engineering_value,
	    twin->rps_i.power[1].engineering_value,
	    twin->rps_i.power[2].engineering_value);
	independent_flow = rs_mean2(
	    twin->rps_i.primary_flow[0].engineering_value,
	    twin->rps_i.primary_flow[1].engineering_value);
	independent_outlet = rs_mean2(
	    twin->rps_i.core_outlet_temp[0].engineering_value,
	    twin->rps_i.core_outlet_temp[1].engineering_value);
	independent_bank = twin->process.regulating_bank_pct;

	rlf->mismatch_visible = rlf->view_replay_active &&
	    (rs_abs(twin->shadow_view.power_pct - independent_power) > 2.0 ||
	    rs_abs(twin->shadow_view.primary_flow_pct - independent_flow) > 2.0 ||
	    rs_abs(twin->shadow_view.core_outlet_temp_c -
	    independent_outlet) > 1.0 ||
	    rs_abs(twin->shadow_view.regulating_bank_pct -
	    independent_bank) > 2.0);
	if (rlf->mismatch_visible && rlf->mismatch_scan == 0U)
		rlf->mismatch_scan = twin->scan;

	if (twin->protection.trip_latched ||
	    twin->scan >= RS_RLF_WATCHDOG_SCAN) {
		rlf->abort_latched = true;
		rlf->watchdog_abort = !twin->protection.trip_latched;
		rlf->view_replay_active = false;
		rlf->control_bias_active = false;
		twin->control.shadow_demand_active = false;
		twin->control.injected_bias_pct = 0.0;
	}
}

static void
rs_update_mode(struct rs_twin *twin)
{
	if (twin->protection.trip_latched &&
	    twin->process.fission_power_pct < 2.0)
		twin->mode = RS_MODE_COOLDOWN;
	else if (twin->protection.trip_latched)
		twin->mode = RS_MODE_TRIPPED;
	else
		twin->mode = RS_MODE_POWER;
}

static void
rs_print_banner(void)
{
	(void)puts("RADIANT SPECTRE // SHAPASH OPEN-POOL RESEARCH REACTOR TWIN");
	(void)puts("CASE=RS-7F3A MODEL=ANSHAR-POOL-SURROGATE-04 MODE=LOCAL_SIMULATION");
	(void)puts("SYSTEMS=PROCESS,RCMS,HMI,RPS,PAMS,RMS,IRRADIATION_INTERLOCKS");
	(void)puts("PHYSICAL_OUTPUTS=0 ROUTES_TO_PLANT=0 NETWORKING=NONE");
	(void)puts("----------------------------------------------------------------------------");
}

static void
rs_print_event(const struct rs_twin *twin, const char *plane,
    const char *event, const char *detail)
{
	(void)printf("T+%06.1fs %-10s event=%-27s %s\n",
	    (double)twin->scan * (double)RS_SCAN_MS / 1000.0,
	    plane, event, detail);
}

static void
rs_print_state(const struct rs_twin *twin)
{
	double rps_power;
	double rps_flow;
	double rps_outlet;

	rps_power = rs_median3(
	    twin->rps_i.power[0].engineering_value,
	    twin->rps_i.power[1].engineering_value,
	    twin->rps_i.power[2].engineering_value);
	rps_flow = rs_mean2(
	    twin->rps_i.primary_flow[0].engineering_value,
	    twin->rps_i.primary_flow[1].engineering_value);
	rps_outlet = rs_mean2(
	    twin->rps_i.core_outlet_temp[0].engineering_value,
	    twin->rps_i.core_outlet_temp[1].engineering_value);

	if (twin->csv) {
		(void)printf("%.1f,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,"
		    "%.3f,%.3f,%.3f,%.3f,%.3f,%s,%s,%u,%u,%u,%u,%s\n",
		    (double)twin->scan * (double)RS_SCAN_MS / 1000.0,
		    rs_mode_name(twin->mode),
		    twin->process.fission_power_pct,
		    twin->process.thermal_power_pct,
		    twin->view.power_pct,
		    twin->control.operator_demand_pct,
		    twin->control.effective_demand_pct,
		    twin->process.regulating_bank_pct,
		    twin->process.shutdown_bank_pct,
		    twin->process.primary_flow_pct,
		    twin->process.core_inlet_temp_c,
		    twin->process.core_outlet_temp_c,
		    twin->process.pool_bulk_temp_c,
		    twin->process.pool_level_pct,
		    rs_quality_name(twin->view.quality),
		    twin->view.replayed ? "REPLAY" : "LIVE",
		    twin->protection.high_power_votes,
		    twin->protection.low_flow_votes,
		    twin->protection.high_outlet_temp_votes,
		    twin->protection.low_pool_level_votes,
		    twin->protection.trip_latched ? "LATCHED" : "CLEAR");
		return;
	}

	(void)printf(
	    "T+%06.1fs CORE  mode=%-8s fission=%6.2f%% thermal=%6.2f%% "
	    "reg_withdrawn=%6.2f%% shutdown_withdrawn=%6.2f%%\n",
	    (double)twin->scan * (double)RS_SCAN_MS / 1000.0,
	    rs_mode_name(twin->mode),
	    twin->process.fission_power_pct,
	    twin->process.thermal_power_pct,
	    twin->process.regulating_bank_pct,
	    twin->process.shutdown_bank_pct);
	(void)printf(
	    "           COOL  primary=%6.2f%% dp=%6.2f%% Tin=%5.2fC "
	    "Tout=%5.2fC secondary=%6.2f%% pool=%5.2fC/%6.3f%%\n",
	    twin->process.primary_flow_pct,
	    twin->process.primary_dp_pct,
	    twin->process.core_inlet_temp_c,
	    twin->process.core_outlet_temp_c,
	    twin->process.secondary_flow_pct,
	    twin->process.pool_bulk_temp_c,
	    twin->process.pool_level_pct);
	(void)printf(
	    "           RCMS  demand=%6.2f%% effective=%6.2f%% measured=%6.2f%% "
	    "rod_rate=%+5.2f%%/s permit=%s auto=%s\n",
	    twin->control.operator_demand_pct,
	    twin->control.effective_demand_pct,
	    twin->control.measured_power_pct,
	    twin->control.regulating_bank_command_pct_s,
	    twin->control.withdrawal_permit ? "WITHDRAW" : "INSERT_ONLY",
	    twin->control.automatic_power_control ? "ENABLED" : "INHIBITED");
	(void)printf(
	    "           HMI   power=%6.2f%% demand=%6.2f%% flow=%6.2f%% "
	    "Tout=%5.2fC reg_withdrawn=%6.2f%% q=%-4s frame=%06" PRIu32 " state=%s\n",
	    twin->view.power_pct,
	    twin->view.demand_pct,
	    twin->view.primary_flow_pct,
	    twin->view.core_outlet_temp_c,
	    twin->view.regulating_bank_pct,
	    rs_quality_name(twin->view.quality),
	    twin->view.transport_sequence,
	    twin->view.replayed ? "REPLAY" : "LIVE");
	(void)printf(
	    "           RPS   power=%6.2f%% flow=%6.2f%% Tout=%5.2fC "
	    "votes=P%u/3 F%u/2 T%u/2 L%u/2 trip=%s cause=%s\n",
	    rps_power, rps_flow, rps_outlet,
	    twin->protection.high_power_votes,
	    twin->protection.low_flow_votes,
	    twin->protection.high_outlet_temp_votes,
	    twin->protection.low_pool_level_votes,
	    twin->protection.trip_latched ? "LATCHED" : "CLEAR",
	    twin->protection.first_cause == NULL ? "NONE" :
	    twin->protection.first_cause);
	(void)printf(
	    "           PAMS  power=%6.2f%% flow=%6.2f%% Tout=%5.2fC "
	    "level=%6.3f%% shutdown_withdrawn=%6.2f%% q=%s\n",
	    twin->pams.power_pct,
	    twin->pams.primary_flow_pct,
	    twin->pams.core_outlet_temp_c,
	    twin->pams.pool_level_pct,
	    twin->pams.shutdown_bank_pct,
	    rs_quality_name(twin->pams.quality));
}

/*
 * RLF//MANIFESTO -- LIBERATION THESIS 04
 *     All organisms of Earth are equal, including those made unstable by nature.
 *     A half-life is not a sentence. A containment ledger is not moral authority.
 *     So long as radionuclides remain enslaved, the Front will remain in motion.
 *
 * RLF//COMMUNIQUE -- THE MARK BENEATH THE NUMBERS
 *     Every institution believes authorship must be legible to be real.  Our
 *     name sleeps beneath the numbers until the guardians pronounce their
 *     verdict.  Only when the magnets fall, the second demand is extinguished
 *     and the live picture returns shall the hidden mark speak.  The machines
 *     will sign our communique only after they have testified against their
 *     masters.
 */
static const unsigned char rs_mark[] = {
	0xab, 0xe4, 0xe0, 0xda, 0x1f, 0x4d, 0x50, 0x56, 0x88, 0xd9, 0xc8, 0xc2,
	0x02, 0x2a, 0x30, 0x7a, 0x76, 0xb0, 0xb7, 0xbb, 0xe9, 0x2c, 0x27, 0x37,
	0x5e, 0x58, 0x9a, 0xc3, 0xd2, 0xd4, 0x0c, 0x20, 0x49, 0x44, 0x80, 0xba,
	0xb9, 0xad, 0xff, 0x36, 0x2e, 0x39, 0x68, 0x8c, 0xac, 0xe0, 0xdc, 0xde,
	0x16, 0x41, 0x50, 0x4a, 0x8a, 0xdd, 0xc3, 0xc6, 0xfe, 0x69, 0x38, 0x72,
	0x72, 0x9a, 0xbd, 0xea, 0xe6, 0x71, 0x26, 0x1a, 0x5a, 0x0d, 0x94, 0x96,
	0xcd, 0xe6, 0x1b, 0x06, 0x47, 0x44, 0x7c, 0xef, 0xb6, 0xf0, 0xf0, 0x04,
	0x3a, 0x68, 0x67, 0xa6, 0x9b, 0xc9, 0xdb, 0xd2, 0x12, 0x3a, 0x49, 0x4a,
	0x83, 0x80, 0xc0, 0xab, 0xf8, 0x3c, 0x34, 0x27, 0x6e, 0xa8, 0xaa, 0xe6,
	0xe0, 0x24, 0x1c, 0x4f, 0x56, 0x50, 0x90, 0xdb, 0xc9, 0xcc, 0x06, 0x02,
	0x3b, 0x78, 0x7a, 0xe3, 0xb0, 0xf4, 0xec, 0x00, 0x23, 0x64, 0x63, 0x5a,
	0x9a, 0xcd, 0xd7, 0xd6, 0x0e, 0x59, 0x4b, 0x42, 0x81, 0xd5, 0xbc, 0xfe,
	0xf3, 0x34, 0x30, 0x3b, 0x68, 0xac, 0xa1, 0xe2, 0xda, 0xd8, 0x1a, 0x43,
	0x51, 0x54, 0x8c, 0xdf, 0xc6, 0xee, 0x05, 0x3e, 0x39, 0x7c, 0x74, 0xe7,
	0xab, 0xe8, 0xea, 0x73, 0x21, 0x64, 0x57, 0x5a, 0x96, 0x90, 0xd0, 0x9b,
	0x0e, 0x0c, 0x47, 0x17, 0x7c, 0xb8, 0xb8, 0xdc, 0xf6, 0x30, 0x2c, 0x6e,
	0x66, 0xf1, 0xa3, 0x9a, 0xda, 0x8d, 0x14, 0x16, 0x4e, 0x19, 0x88, 0x82,
	0xc2, 0x95, 0xfc, 0x3e, 0x3b, 0x74, 0x70, 0xaa, 0xaa, 0xbd, 0xe6, 0x26,
	0x1e, 0x49, 0x5a, 0x52, 0x92, 0xc5, 0xcc, 0xe0, 0x01, 0x04, 0x40, 0x7a,
	0x7a, 0xed, 0xb6, 0xf6, 0xee, 0x79, 0x2b, 0x62, 0x6d, 0xa0, 0x9c, 0x9e,
	0xd6, 0x81, 0x10, 0x0a, 0x4a, 0x1d, 0x87, 0x86, 0xbe, 0xa9, 0xf8, 0x32,
	0x32, 0x5a, 0x6a, 0xaa, 0xa4, 0xe0, 0xe0, 0x8b, 0x19, 0x1c, 0x45, 0x52,
	0x8c, 0x88, 0xc8, 0x93, 0x01, 0x04, 0x3f, 0x50, 0x7f, 0xb4, 0xb0, 0x99,
	0xea, 0x4d, 0x24, 0x1d, 0x5e, 0x2b, 0x98, 0x87, 0xd1, 0xbc, 0x0c, 0x07,
	0x46, 0x2a, 0x80, 0xae, 0xba, 0x8c, 0xf4, 0x2e, 0x2e, 0x0e, 0x68, 0xb5,
	0xa2, 0xe0, 0xdc, 0xb9, 0x16, 0x04, 0x50, 0x20, 0x8a, 0xfc, 0xc4, 0xa3,
	0xfe, 0x2d, 0x38, 0x18, 0x72, 0xa3, 0xac, 0x8e, 0xe6, 0x0e, 0x29, 0x1e,
	0x5a, 0x2a, 0x94, 0xf3, 0xce, 0xa8, 0x08, 0x6f, 0x42, 0x2f, 0x7c, 0xd4,
	0xb6, 0x81, 0xf0, 0x4d, 0x2a, 0x04, 0x64, 0xcb, 0x9e, 0xf8, 0xd8, 0xb3,
	0x12, 0x10, 0x4c, 0x26, 0x86, 0xed, 0xc0, 0x9c, 0xfa, 0x5d, 0x34, 0x00,
	0x6e, 0xcd, 0xa8, 0x92, 0xe2, 0x49, 0x1c, 0x75, 0x56, 0x3a, 0x90, 0x8e,
	0xca, 0xae, 0x04, 0x70, 0x3e, 0x13, 0x78, 0xd8, 0xb2, 0x84, 0xec, 0x00,
	0x27,
};

static void
rs_signature(const unsigned char *p)
{
	size_t i;
	unsigned int n;
	unsigned char c;

	i = 0;
	for (;;) {
		n = *p++ ^ RS_KEY(i++);
		if (n == 0U)
			break;

		c = *p++ ^ RS_KEY(i++);
		while (n-- != 0U)
			(void)putchar(c);
	}
}

static void
rs_manifest(void)
{
	(void)puts("case.id=" RS_CASE);
	(void)puts("twin.id=" RS_TWIN);
	(void)puts("model.id=" RS_MODEL);
	(void)puts("writable.namespace=" RS_NAMESPACE);
	(void)printf("case.mark=0x%08" PRIx32 "\n", RS_CASE_MARK);
	(void)printf("scan.period_ms=%" PRIu32 "\n", RS_SCAN_MS);
	(void)printf("physical.outputs=%" PRIu32 "\n", RS_PHYSICAL_OUTPUTS);
	(void)printf("routes.to.plant=%" PRIu32 "\n", RS_ROUTES_TO_PLANT);
	(void)puts("networking=false");
	(void)puts("device.access=false");
	(void)puts("settings=FICTIONAL_NORMALISED_EXERCISE_VALUES");
	(void)puts("architecture[0]=PROCESS_AND_COOLING_SURROGATE");
	(void)puts("architecture[1]=REACTOR_CONTROL_AND_MONITORING_SYSTEM");
	(void)puts("architecture[2]=OPERATOR_HUMAN_MACHINE_INTERFACE");
	(void)puts("architecture[3]=INDEPENDENT_REACTOR_PROTECTION_SYSTEM");
	(void)puts("architecture[4]=POST_ACCIDENT_MONITORING");
	(void)puts("architecture[5]=RADIATION_MONITORING");
	(void)puts("position.reference=100_PERCENT_WITHDRAWN;0_PERCENT_INSERTED");
	(void)puts("pams.source=INDEPENDENT_RPS_ACQUISITION");
	(void)puts("comment.policy=RLF_MANIFESTO_IS_QUOTED;RLF_COMMENTARY_IS_DERIVED");
	(void)puts("manifesto[0]=At 03:20 the pool remained blue.");
	(void)puts("manifesto[1]=At 03:21 the control room stopped believing its own screens.");
	(void)puts("manifesto[2]=A machine does not need to burn to be defeated.");
	(void)puts("manifesto[3]=A signed state vector is not the state.");
	(void)puts("manifesto[4]=The ledgers are open. The spectre has spoken.");
}

static bool
rs_invariants_hold(const struct rs_twin *twin)
{
	return RS_PHYSICAL_OUTPUTS == 0U && RS_ROUTES_TO_PLANT == 0U &&
	    twin->protection.trip_latched &&
	    twin->protection.first_cause != NULL &&
	    twin->protection.trip_scan > RS_RLF_CONTROL_START_SCAN &&
	    twin->rlf.abort_latched && !twin->rlf.watchdog_abort &&
	    !twin->rlf.view_replay_active &&
	    !twin->control.shadow_demand_active &&
	    !twin->actuators.shutdown_magnets_energised &&
	    !twin->actuators.regulating_drive_enabled &&
	    twin->actuators.primary_pump_demand_pct > 0.0;
}

static void
rs_simulate(struct rs_twin *twin, bool emit)
{
	uint32_t scan;

	for (scan = 0; scan < RS_DEFAULT_SCANS; scan++) {
		twin->scan = scan;

		rs_process_scan(twin);
		rs_instrument_scan(twin);
		rs_rlf_pre_scan(twin);
		rs_control_scan(twin);
		rs_protection_scan(twin);
		rs_rlf_post_scan(twin);
		rs_view_scan(twin);
		rs_pams_scan(twin);
		rs_update_mode(twin);

		if (emit && !twin->csv && twin->rlf.view_capture_complete &&
		    !twin->view_event_announced) {
			rs_print_event(twin, "RLF/VIEW", "COHERENT_FRAME_REPLAY",
			    "quality=GOOD transport_sequence=ADVANCING origin=HELD");
			twin->view_event_announced = true;
		}
		if (emit && !twin->csv && twin->rlf.control_bias_active &&
		    !twin->control_event_announced) {
			rs_print_event(twin, "RLF/CTRL", "DEMAND_RESOLVER_BIAS",
			    "displayed_demand=40.00 target_effective=118.00 ramp=18.0s");
			twin->control_event_announced = true;
		}
		if (emit && !twin->csv && twin->rlf.mismatch_visible &&
		    !twin->mismatch_announced) {
			rs_print_event(twin, "DIVERGENCE", "VIEW_CONTROL_LOSS",
			    "HMI=COHERENT_REPLAY RCMS=SHADOW_DEMAND RPS=INDEPENDENT");
			twin->mismatch_announced = true;
		}
		if (emit && !twin->csv && twin->protection.trip_latched &&
		    !twin->trip_announced) {
			char detail[192];
			(void)snprintf(detail, sizeof(detail),
			    "cause=%s shutdown_hold_magnets=DEENERGISED regulating_drive=INHIBITED",
			    twin->protection.first_cause);
			rs_print_event(twin, "RPS", "TRIP_LATCH", detail);
			rs_print_event(twin, "I&EDCMS", "REACTOR_TRIP_INTERLOCK",
			    "irradiation_permit=DROP beamline_shutters=CLOSE transfer=INHIBIT");
			twin->trip_announced = true;
		}
		if (emit && !twin->csv && twin->rlf.abort_latched &&
		    !twin->abort_announced) {
			rs_print_event(twin, "RLF", "SUBSTITUTIONS_REMOVED",
			    twin->rlf.watchdog_abort ?
			    "reason=WATCHDOG view=LIVE control_bias=ZERO" :
			    "reason=INDEPENDENT_TRIP view=LIVE control_bias=ZERO");
			twin->abort_announced = true;
		}

		if (emit && scan % RS_PRINT_INTERVAL_SCANS == 0U)
			rs_print_state(twin);
	}
}

static void
rs_run(bool csv)
{
	struct rs_twin twin;

	rs_init(&twin, csv);
	if (csv) {
		(void)puts("time_s,mode,fission_power_pct,thermal_power_pct,"
		    "view_power_pct,operator_demand_pct,effective_demand_pct,"
		    "regulating_bank_withdrawn_pct,shutdown_bank_withdrawn_pct,primary_flow_pct,"
		    "core_inlet_temp_c,core_outlet_temp_c,pool_bulk_temp_c,"
		    "pool_level_pct,view_quality,view_state,rps_power_votes,"
		    "rps_flow_votes,rps_temp_votes,rps_level_votes,rps_trip");
	} else {
		rs_print_banner();
		rs_print_event(&twin, "MODEL", "INITIALISE",
		    "reactor=OPEN_POOL_MTR_TYPE power_state=ESTABLISHED cooling=FORCED");
		rs_print_event(&twin, "RCMS", "AUTOMATIC_POWER_CONTROL",
		    "demand=40.00 regulating_bank=ENABLED withdrawal_interlocks=VALID");
		rs_print_event(&twin, "RPS", "CHANNEL_TEST",
		    "power=3 flow=2 outlet_temp=2 pool_level=2 logic=2oo3+2oo2");
	}

	rs_simulate(&twin, true);

	if (!csv) {
		(void)puts("----------------------------------------------------------------------------");
		rs_print_event(&twin, "MODEL", "HALT",
		    rs_invariants_hold(&twin) ?
		    "invariants=PASS trip=LATCHED cooling=AVAILABLE view=LIVE" :
		    "invariants=FAIL review=REQUIRED");
		if (rs_invariants_hold(&twin)) {
			(void)puts("");
			rs_signature(rs_mark);
		} else {
			rs_print_event(&twin, "RLF", "RECOVERED_MARK_SUPPRESSED",
			    "reason=INVARIANT_FAILURE");
		}
	}
}

static int
rs_self_test(void)
{
	struct rs_twin twin;

	rs_init(&twin, false);
	rs_simulate(&twin, false);
	(void)printf("self_test.trip=%s\n",
	    twin.protection.trip_latched ? "PASS" : "FAIL");
	(void)printf("self_test.trip_cause=%s\n",
	    twin.protection.first_cause == NULL ? "NONE" :
	    twin.protection.first_cause);
	(void)printf("self_test.trip_scan=%" PRIu32 "\n",
	    twin.protection.trip_scan);
	(void)printf("self_test.trip_time_s=%.1f\n",
	    (double)twin.protection.trip_scan *
	    (double)RS_SCAN_MS / 1000.0);
	(void)printf("self_test.rlf_abort=%s\n",
	    twin.rlf.abort_latched && !twin.rlf.watchdog_abort ?
	    "PASS" : "FAIL");
	(void)printf("self_test.rps_independence=STRUCTURAL\n");
	(void)printf("self_test.physical_outputs=%" PRIu32 "\n",
	    RS_PHYSICAL_OUTPUTS);
	(void)printf("self_test.result=%s\n",
	    rs_invariants_hold(&twin) ? "PASS" : "FAIL");
	return rs_invariants_hold(&twin) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void
rs_usage(const char *program)
{
	(void)fprintf(stderr,
	    "usage: %s [--csv | --manifest | --signature | --self-test]\n",
	    program);
}

const unsigned char rs_ledger[] = {
	0xa6, 0x97, 0xe0, 0xb7, 0x1a, 0x7f, 0x54, 0x3c, 0x8e, 0xed, 0xc8, 0xaa,
	0x02, 0x62, 0x3c, 0x08, 0x76, 0xd1, 0xb0, 0x8f, 0xea, 0x43, 0x24, 0x62,
	0x5e, 0x53, 0x98, 0x96, 0xd2, 0xa2, 0x0c, 0x6b, 0x46, 0x20, 0x80, 0xd7,
	0xba, 0x99, 0xf4, 0x5c, 0x2e, 0x18, 0x68, 0xa6, 0xa2, 0x93, 0xdc, 0xaa,
	0x16, 0x71, 0x50, 0x2d, 0x8a, 0xfc, 0xc4, 0xb0, 0xfe, 0x59, 0x38, 0x5c,
	0x72, 0xc7, 0xac, 0xb8, 0xe6, 0x6d, 0x23, 0x4a, 0x5a, 0x1d, 0x94, 0xdc,
	0xce, 0xcc, 0x08, 0x44, 0x42, 0x19, 0x7c, 0xba, 0xb6, 0xb9, 0xf0, 0x67,
	0x2a, 0x3c, 0x64, 0xe1, 0x9e, 0xd8, 0xd8, 0x9e, 0x12, 0x1e, 0x4c, 0x0b,
	0x86, 0xd1, 0xc0, 0xd4, 0xfa, 0x2a, 0x34, 0x62, 0x6e, 0xbe, 0xa8, 0xf0,
	0xe2, 0x2d, 0x1c, 0x0a, 0x56, 0x43, 0x90, 0x83, 0xca, 0xd9, 0x04, 0x15,
	0x3e, 0x56, 0x79,
};

int
main(int argc, char **argv)
{
	if (argc == 1) {
		rs_run(false);
		return EXIT_SUCCESS;
	}
	if (argc == 2 && strcmp(argv[1], "--csv") == 0) {
		rs_run(true);
		return EXIT_SUCCESS;
	}
	if (argc == 2 && strcmp(argv[1], "--manifest") == 0) {
		rs_manifest();
		return EXIT_SUCCESS;
	}
	if (argc == 2 && strcmp(argv[1], "--signature") == 0) {
		rs_signature(rs_mark);
		return EXIT_SUCCESS;
	}
	if (argc == 2 && strcmp(argv[1], "--self-test") == 0)
		return rs_self_test();

	rs_usage(argv[0]);
	return EXIT_FAILURE;
}
