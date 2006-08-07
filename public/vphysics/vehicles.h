//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VEHICLES_H
#define VEHICLES_H
#ifdef _WIN32
#pragma once
#endif

#include "datamap.h"


#define VEHICLE_TYPE_CAR_WHEELS			(1<<0)
#define VEHICLE_TYPE_CAR_RAYCAST		(1<<1)
#define VEHICLE_TYPE_JETSKI_RAYCAST		(1<<2)
#define VEHICLE_TYPE_AIRBOAT_RAYCAST	(1<<3)

#define VEHICLE_MAX_AXLE_COUNT	4
#define VEHICLE_MAX_GEAR_COUNT	6
#define VEHICLE_MAX_WHEEL_COUNT	(2*VEHICLE_MAX_AXLE_COUNT)

#define VEHICLE_TIRE_NORMAL		0
#define VEHICLE_TIRE_BRAKING	1
#define VEHICLE_TIRE_POWERSLIDE	2

struct vehicle_controlparams_t
{
	float	throttle;
	float	steering;
	float	brake;
	float	boost;
	bool	handbrake;
	bool	handbrakeLeft;
	bool	handbrakeRight;
	bool	brakepedal;
	bool	bHasBrakePedal;
};

struct vehicle_operatingparams_t
{
	DECLARE_SIMPLE_DATADESC();

	float	speed;
	float	engineRPM;
	int		gear;
	float	boostDelay;
	int		boostTimeLeft;
	bool	skidding;
	float	wheelSkidValue[VEHICLE_MAX_WHEEL_COUNT];
};

// Debug!
#define VEHICLE_DEBUGRENDERDATA_MAX_WHEELS		10
#define VEHICLE_DEBUGRENDERDATA_MAX_AXLES		3

struct vehicle_debugcarsystem_t
{
	Vector vecAxlePos[VEHICLE_DEBUGRENDERDATA_MAX_AXLES];

	Vector vecWheelPos[VEHICLE_DEBUGRENDERDATA_MAX_WHEELS];
	Vector vecWheelRaycasts[VEHICLE_DEBUGRENDERDATA_MAX_WHEELS][2];
	Vector vecWheelRaycastImpacts[VEHICLE_DEBUGRENDERDATA_MAX_WHEELS];
};

struct vehicleparams_t;

class IPhysicsVehicleController
{
public:
	virtual ~IPhysicsVehicleController() {}
	// call this from the game code with the control parameters
	virtual void Update( float dt, vehicle_controlparams_t &controls ) = 0;
	virtual const vehicle_operatingparams_t &GetOperatingParams() = 0;
	virtual const vehicleparams_t &GetVehicleParams() = 0;
	virtual vehicleparams_t &GetVehicleParamsForChange() = 0;
	virtual float UpdateBooster(float dt) = 0;
	virtual int GetWheelCount(void) = 0;
	virtual IPhysicsObject *GetWheel(int index) = 0;
	virtual void GetWheelContactPoint( int index, Vector &contact ) = 0;
	virtual float GetWheelSkidDeltaTime( int index ) = 0;
	virtual void SetSpringLength(int wheelIndex, float length) = 0;
	virtual void SetWheelFriction(int wheelIndex, float friction) = 0;

	virtual void OnVehicleEnter( void ) = 0;
	virtual void OnVehicleExit( void ) = 0;

	virtual void SetEngineDisabled( bool bDisable ) = 0;
	virtual bool IsEngineDisabled( void ) = 0;

	// Debug
	virtual void GetCarSystemDebugData( vehicle_debugcarsystem_t &debugCarSystem ) = 0;
};


// parameters for the body object control of the vehicle
struct vehicle_bodyparams_t
{
	DECLARE_SIMPLE_DATADESC();

	Vector		massCenterOverride;		// leave at vec3_origin for no override
	float		massOverride;			// leave at 0 for no override
	float		addGravity;				// keeps car down
	float		tiltForce;				// keeps car down when not on flat ground
	float		tiltForceHeight;		// where the tilt force pulls relative to center of mass
	float		counterTorqueFactor;
	float		keepUprightTorque;
};

// wheel objects are created by vphysics, these are the parameters for those objects
// NOTE: They are paired, so only one set of parameters is necessary per axle
struct vehicle_wheelparams_t
{
	DECLARE_SIMPLE_DATADESC();

	float		radius;
	float		mass;
	float		inertia;
	float		damping;		// usually 0
	float		rotdamping;		// usually 0
	float		frictionScale;	// 1.5 front, 1.8 rear
	int			materialIndex;
	int			brakeMaterialIndex;
	int			skidMaterialIndex;
	float		springAdditionalLength;	// 0 means the spring is at it's rest length
};

struct vehicle_suspensionparams_t
{
	DECLARE_SIMPLE_DATADESC();

	float		springConstant;
	float		springDamping;
	float		stabilizerConstant;
	float		springDampingCompression;
	float		maxBodyForce;
};

// NOTE: both raytrace and wheel data here because jetski uses both.
struct vehicle_axleparams_t
{
	DECLARE_SIMPLE_DATADESC();

	Vector						offset;					// center of this axle in vehicle object space
	Vector						wheelOffset;			// offset to wheel (assume other wheel is symmetric at -wheelOffset) from axle center
	Vector						raytraceCenterOffset;	// offset to center of axle for the raytrace data.
	Vector						raytraceOffset;			// offset to raytrace for non-wheel (some wheeled) vehicles
	vehicle_wheelparams_t		wheels;
	vehicle_suspensionparams_t	suspension;
	float						torqueFactor;		// normalized to 1 across all axles 
													// e.g. 0,1 for rear wheel drive - 0.5,0.5 for 4 wheel drive
	float						brakeFactor;		// normalized to 1 across all axles
};

struct vehicle_steeringparams_t
{
	DECLARE_SIMPLE_DATADESC();

	float		degrees;				// angle in degrees of steering
	float		steeringRateSlow;		// this is the speed the wheels are steered when the vehicle is slow
	float		steeringRateFast;		// this is the speed the wheels are steered when the vehicle is "fast"
	float		steeringRestFactor;		// this is the speed at which the wheels move toward their resting state (straight ahead)
	float		speedSlow;				// this is the max speed of "slow"
	float		speedFast;				// this is the min speed of "fast"
	bool		isSkidAllowed;			// true/false skid flag
	bool		dustCloud;				// flag for creating a dustcloud behind vehicle
};

struct vehicle_engineparams_t
{
	DECLARE_SIMPLE_DATADESC();

	float		horsepower;
	float		maxSpeed;
	float		maxRevSpeed;
	float		maxRPM;					// redline RPM limit
	float		axleRatio;				// ratio of engine rev to axle rev
	float		throttleTime;			// time to reach full throttle in seconds

	// transmission
	bool		isAutoTransmission;		// true for auto, false for manual
	int			gearCount;				// gear count - max 10
	float		gearRatio[VEHICLE_MAX_GEAR_COUNT];	// ratio for each gear

	// automatic transmission (simple auto-shifter - switches at fixed RPM limits)
	float		shiftUpRPM;				// max RPMs to switch to a higher gear
	float		shiftDownRPM;			// min RPMs to switch to a lower gear
	float		boostForce;
	float		boostDuration;
	float		boostDelay;
	float		boostMaxSpeed;
	bool		torqueBoost;
};

struct vehicleparams_t
{
	DECLARE_SIMPLE_DATADESC();

	int							axleCount;
	int							wheelsPerAxle;
	vehicle_bodyparams_t		body;
	vehicle_axleparams_t		axles[VEHICLE_MAX_AXLE_COUNT];
	vehicle_engineparams_t		engine;
	vehicle_steeringparams_t	steering;
};

// Iterator for queries
class CPassengerSeatTransition;
typedef CUtlVector< CPassengerSeatTransition> PassengerSeatAnims_t;

// Seat query types
enum VehicleSeatQuery_e
{
	VEHICLE_SEAT_ANY,			// Any available seat for our role
	VEHICLE_SEAT_NEAREST,		// Seat closest to our starting point
};

// Seat anim types for return
enum PassengerSeatAnimType_t
{
	PASSENGER_SEAT_ENTRY,
	PASSENGER_SEAT_EXIT
};

#define VEHICLE_SEAT_INVALID	-1		// An invalid seat

#endif // VEHICLES_H
