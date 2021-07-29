/**
 *  Adapted from Penner Easing equations' C++ port.
 *  Source: https://github.com/jesusgollonet/ofpennereasing
 *  License: BSD-3-clause
 */

#include "scene/animation/tween.h"

const real_t pi = 3.1415926535898;

///////////////////////////////////////////////////////////////////////////
// linear
///////////////////////////////////////////////////////////////////////////
namespace linear {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	return c * t / d + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	return c * t / d + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	return c * t / d + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return c * t / d + b;
}
}; // namespace linear
///////////////////////////////////////////////////////////////////////////
// sine
///////////////////////////////////////////////////////////////////////////
namespace sine {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	return -c * cos(t / d * (pi / 2)) + c + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	return c * sin(t / d * (pi / 2)) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	return -c / 2 * (cos(pi * t / d) - 1) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace sine
///////////////////////////////////////////////////////////////////////////
// quint
///////////////////////////////////////////////////////////////////////////
namespace quint {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	return c * pow(t / d, 5) + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	return c * (pow(t / d - 1, 5) + 1) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d * 2;
	if (t < 1) return c / 2 * pow(t, 5) + b;
	return c / 2 * (pow(t - 2, 5) + 2) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace quint
///////////////////////////////////////////////////////////////////////////
// quart
///////////////////////////////////////////////////////////////////////////
namespace quart {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	return c * pow(t / d, 4) + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	return -c * (pow(t / d - 1, 4) - 1) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d * 2;
	if (t < 1) return c / 2 * pow(t, 4) + b;
	return -c / 2 * (pow(t - 2, 4) - 2) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace quart
///////////////////////////////////////////////////////////////////////////
// quad
///////////////////////////////////////////////////////////////////////////
namespace quad {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	return c * pow(t / d, 2) + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d;
	return -c * t * (t - 2) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d * 2;
	if (t < 1) return c / 2 * pow(t, 2) + b;
	return -c / 2 * ((t - 1) * (t - 3) - 1) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace quad
///////////////////////////////////////////////////////////////////////////
// expo
///////////////////////////////////////////////////////////////////////////
namespace expo {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0) return b;
	return c * pow(2, 10 * (t / d - 1)) + b - c * 0.001;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	if (t == d) return b + c;
	return c * 1.001 * (-pow(2, -10 * t / d) + 1) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0) return b;
	if (t == d) return b + c;
	t = t / d * 2;
	if (t < 1) return c / 2 * pow(2, 10 * (t - 1)) + b - c * 0.0005;
	return c / 2 * 1.0005 * (-pow(2, -10 * (t - 1)) + 2) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace expo
///////////////////////////////////////////////////////////////////////////
// elastic
///////////////////////////////////////////////////////////////////////////
namespace elastic {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0) return b;
	if ((t /= d) == 1) return b + c;
	float p = d * 0.3f;
	float a = c;
	float s = p / 4;
	float postFix = a * pow(2, 10 * (t -= 1)); // this is a fix, again, with post-increment operators
	return -(postFix * sin((t * d - s) * (2 * pi) / p)) + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0) return b;
	if ((t /= d) == 1) return b + c;
	float p = d * 0.3f;
	float a = c;
	float s = p / 4;
	return (a * pow(2, -10 * t) * sin((t * d - s) * (2 * pi) / p) + c + b);
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0) return b;
	if ((t /= d / 2) == 2) return b + c;
	float p = d * (0.3f * 1.5f);
	float a = c;
	float s = p / 4;

	if (t < 1) {
		float postFix = a * pow(2, 10 * (t -= 1)); // postIncrement is evil
		return -0.5f * (postFix * sin((t * d - s) * (2 * pi) / p)) + b;
	}
	float postFix = a * pow(2, -10 * (t -= 1)); // postIncrement is evil
	return postFix * sin((t * d - s) * (2 * pi) / p) * 0.5f + c + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace elastic
///////////////////////////////////////////////////////////////////////////
// cubic
///////////////////////////////////////////////////////////////////////////
namespace cubic {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	t /= d;
	return c * t * t * t + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d - 1;
	return c * (t * t * t + 1) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	t /= d / 2;
	if (t < 1) return c / 2 * t * t * t + b;
	t -= 2;
	return c / 2 * (t * t * t + 2) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace cubic
///////////////////////////////////////////////////////////////////////////
// circ
///////////////////////////////////////////////////////////////////////////
namespace circ {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	t /= d;
	return -c * (sqrt(1 - t * t) - 1) + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d - 1;
	return c * sqrt(1 - t * t) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	t /= d / 2;
	if (t < 1) {
		return -c / 2 * (sqrt(1 - t * t) - 1) + b;
	}
	t -= 2;
	return c / 2 * (sqrt(1 - t * t) + 1) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace circ
///////////////////////////////////////////////////////////////////////////
// bounce
///////////////////////////////////////////////////////////////////////////
namespace bounce {
static real_t out(real_t t, real_t b, real_t c, real_t d);

static real_t in(real_t t, real_t b, real_t c, real_t d) {
	return c - out(d - t, 0, c, d) + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	if ((t /= d) < (1 / 2.75f)) {
		return c * (7.5625f * t * t) + b;
	} else if (t < (2 / 2.75f)) {
		float postFix = t -= (1.5f / 2.75f);
		return c * (7.5625f * (postFix)*t + .75f) + b;
	} else if (t < (2.5 / 2.75)) {
		float postFix = t -= (2.25f / 2.75f);
		return c * (7.5625f * (postFix)*t + .9375f) + b;
	} else {
		float postFix = t -= (2.625f / 2.75f);
		return c * (7.5625f * (postFix)*t + .984375f) + b;
	}
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? in(t * 2, b, c / 2, d) : out((t * 2) - d, b + c / 2, c / 2, d);
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace bounce
///////////////////////////////////////////////////////////////////////////
// back
///////////////////////////////////////////////////////////////////////////
namespace back {
static real_t in(real_t t, real_t b, real_t c, real_t d) {
	float s = 1.70158f;
	float postFix = t /= d;
	return c * (postFix)*t * ((s + 1) * t - s) + b;
}

static real_t out(real_t t, real_t b, real_t c, real_t d) {
	float s = 1.70158f;
	t = t / d - 1;
	return c * (t * t * ((s + 1) * t + s) + 1) + b;
}

static real_t in_out(real_t t, real_t b, real_t c, real_t d) {
	float s = 1.70158f * 1.525f;
	t /= d / 2;
	if (t < 1) return c / 2 * (t * t * ((s + 1) * t - s)) + b;
	t -= 2;
	return c / 2 * (t * t * ((s + 1) * t + s) + 2) + b;
}

static real_t out_in(real_t t, real_t b, real_t c, real_t d) {
	return (t < d / 2) ? out(t * 2, b, c / 2, d) : in((t * 2) - d, b + c / 2, c / 2, d);
}
}; // namespace back

Tween::interpolater Tween::interpolaters[Tween::TRANS_COUNT][Tween::EASE_COUNT] = {
	{ &linear::in, &linear::out, &linear::in_out, &linear::out_in },
	{ &sine::in, &sine::out, &sine::in_out, &sine::out_in },
	{ &quint::in, &quint::out, &quint::in_out, &quint::out_in },
	{ &quart::in, &quart::out, &quart::in_out, &quart::out_in },
	{ &quad::in, &quad::out, &quad::in_out, &quad::out_in },
	{ &expo::in, &expo::out, &expo::in_out, &expo::out_in },
	{ &elastic::in, &elastic::out, &elastic::in_out, &elastic::out_in },
	{ &cubic::in, &cubic::out, &cubic::in_out, &cubic::out_in },
	{ &circ::in, &circ::out, &circ::in_out, &circ::out_in },
	{ &bounce::in, &bounce::out, &bounce::in_out, &bounce::out_in },
	{ &back::in, &back::out, &back::in_out, &back::out_in },
};

real_t Tween::_run_equation(TransitionType p_trans_type, EaseType p_ease_type, real_t t, real_t b, real_t c, real_t d) {

	interpolater cb = interpolaters[p_trans_type][p_ease_type];
	ERR_FAIL_COND_V(cb == NULL, b);
	return cb(t, b, c, d);
}
