//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "npc_fountain.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
//
// CNPC_BlobFountain
//

//---------------------------------------------------------
// Custom Client entity
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_BlobFountain, DT_NPC_BlobFountain)

END_SEND_TABLE()

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------

BEGIN_DATADESC( CNPC_BlobFountain )
	DEFINE_AUTO_ARRAY(  m_bContact, FIELD_BOOLEAN ),

END_DATADESC()




LINK_ENTITY_TO_CLASS( npc_blob_fountain, CNPC_BlobFountain );


#if 1

IMotionEvent::simresult_e CBlobFountainController::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	if(CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI)
	{
		pObject->EnableMotion(false);
		return IMotionEvent::SIM_NOTHING;
	}


	//if(m_pOwner->m_bPause) return IMotionEvent::SIM_NOTHING;
	//if(CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI) return IMotionEvent::SIM_NOTHING;
	/*
	bool newPause = CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI;

	if

	if(newPause != m_bPause)
	{
	if(newPause)
	{
	for (int i = 0; i < m_nActiveParticles; i++)
	{
	m_vecPhysParticles[i]->EnableMotion(false);
	}
	}
	else
	{
	for (int i = 0; i < m_nActiveParticles; i++)
	{
	m_vecPhysParticles[i]->EnableMotion(true);
	}
	}
	m_bPause = newPause;
	}

	if(m_bPause) return;
	*/



	int nSphere = pObject->GetGameIndex();
#if 0
	if (m_pOwner && m_flLennardJonesTime != gpGlobals->curtime)
	{
		m_pOwner->Simulate( pController, pObject, deltaTime, linear, angular );
		m_flLennardJonesTime = gpGlobals->curtime;
	}

	linear += m_vecLennardJonesForce[nSphere] * (1.0f / deltaTime);
#endif

#if 1
	Vector pos, vecVel;
	QAngle ang;
	pObject->GetPosition( &pos, NULL );
	pObject->GetVelocity( &vecVel, NULL );
	float d = sv_surface_radius.GetFloat();
	Vector nozzle = m_pOwner->GetNozzle();
	Vector start = nozzle + Vector(0.0f,0.0f,d);
	pos = pos - nozzle;
	float dist = pos.Length();

	if(m_pOwner->m_iMode[nSphere] == 0)
	{
		if (dist < 10.0f)
		{
			if(vecVel.z < 0.1f && abs(vecVel.x) < 0.05f && abs(vecVel.y) < 0.05f)
			{
				m_pOwner->m_iContactTime[nSphere] = 0;
				m_pOwner->m_fRadius[nSphere] = 0.5f;
				m_pOwner->m_iMode[nSphere] = 1;
				m_pOwner->m_bContact[nSphere] = false;
				//linear.z += 10000 * (1.0f / deltaTime);
				//linear.z = 500000.0f;
				//linear = -vecVel / deltaTime;

				float v = 6000.0f / deltaTime;
				float azimuth = (2.0f * M_PI * (float(rand()) / float(VALVE_RAND_MAX)));
				float polar = 0.04f * M_PI + (0.01f * (float(rand()) / float(VALVE_RAND_MAX)));
				//static float azimuth = 0.0f;
				//azimuth += (0.05f * M_PI);
				//float cur_azimuth = azimuth + RandomFloat(-0.01, 0.01);

				linear.x = v * cos(azimuth) * sin(polar);
				linear.y = v * sin(azimuth) * sin(polar);
				linear.z = v * cos(polar);
			}
			//else
			//{
			//	pObject->SetVelocity( &Vector(0.0f, 0.0f, 0.0f), NULL );
			//}
		}
	}
	else if(m_pOwner->m_iMode[nSphere] == 1)
	{
		m_pOwner->m_fRadius[nSphere] = min(1.0f, m_pOwner->m_fRadius[nSphere] + 3.0f*deltaTime);
		if(m_pOwner->m_bContact[nSphere]) // vecVel.z < 0.1f && 
		{
			if(dist < 80.0f)
			{
				m_pOwner->m_fRadius[nSphere] = 0.0f;
				pObject->SetPosition(nozzle, QAngle(0.0f, 0.0f, 0.0f), true);
				Vector zero(0.0f, 0.0f, 0.0f);
				pObject->SetVelocity( &zero, NULL );
				m_pOwner->m_iMode[nSphere] = 0;
				m_pOwner->m_iContactTime[nSphere] = 0;
			}
			else
			{
				m_pOwner->m_fRadius[nSphere] = 0.0f; //max(0.0f, m_pOwner->m_fRadius[nSphere] - 2.0f*deltaTime);
				m_pOwner->m_iContactTime[nSphere] += deltaTime;
				if(m_pOwner->m_iContactTime[nSphere] >= 1.0f)
				{
					m_pOwner->m_iMode[nSphere] = 2;
					m_pOwner->m_iContactTime[nSphere] = 0;
					m_pOwner->m_bContact[nSphere] = false;
				}
			}
		}
	}
	else if(m_pOwner->m_iMode[nSphere] == 2)
	{
		pObject->SetPosition(nozzle, QAngle(0.0f, 0.0f, 0.0f), true);
		Vector zero(0.0f, 0.0f, 0.0f);
		pObject->SetVelocity( &zero, NULL );
		//Vector vVelocity = Vector( RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ) ) * 0.2f;
		//pObject->SetVelocity( &vVelocity, NULL );

		m_pOwner->m_iContactTime[nSphere] += deltaTime;
		if(m_pOwner->m_iContactTime[nSphere] >= 1.0f)
		{
			m_pOwner->m_iMode[nSphere] = 0;
			m_pOwner->m_iContactTime[nSphere] = 0;
		}
	}

#if 0
	if(m_pOwner->m_bContact[nSphere]) // || (vecVel.z < -20.0f && pos.z < 40.0f))
	{
		m_pOwner->m_flSurfaceR[nSphere] = 0.0f; //max(0.0f, m_pOwner->m_flSurfaceR[nSphere] - 0.1f);
		m_pOwner->m_iContactTime[nSphere] ++;

		//if(m_pOwner->m_flSurfaceR[nSphere] <= 0.0f)
		if(m_pOwner->m_iContactTime[nSphere] >= 10)
		{
			m_pOwner->m_bContact[nSphere] = false;

			if(dist > 10.0f)
			{
				pObject->SetPosition(nozzle, QAngle(0.0f, 0.0f, 0.0f), true);
				Vector vVelocity = Vector( RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ) ) * 0.2f;
				pObject->SetVelocity( &vVelocity, NULL );
				//m_pOwner->m_flSurfaceR[nSphere] = 0.1f


				//Vector vVelocity(0.0f, 0.0f, 0.0f);
				//pObject->SetVelocity( &vVelocity, NULL );
			}
		}
	}
	else
	{
		//pObject->SetVelocity( &Vector(), &AngularImpulse());
		if (dist < 10.0f)
		{
			if(vecVel.z < 0.1f && abs(vecVel.x) < 0.1f && abs(vecVel.y) < 0.1f)
			{
				m_pOwner->m_iContactTime[nSphere] = 0;
				m_pOwner->m_flSurfaceR[nSphere] = 1.0f;
				//linear.z += 10000 * (1.0f / deltaTime);
				//linear.z = 500000.0f;
				//linear = -vecVel / deltaTime;

				float v = 6000.0f / deltaTime;
				float azimuth = (2.0f * M_PI * (float(rand()) / float(VALVE_RAND_MAX)));
				float polar = 0.03f * M_PI; //(0.03f * M_PI * (float(rand()) / float(VALVE_RAND_MAX))); // 

				linear.x = v * cos(azimuth) * sin(polar);
				linear.y = v * sin(azimuth) * sin(polar);
				linear.z = v * cos(polar);
			}
		}
	}
#endif
#endif

	return IMotionEvent::SIM_GLOBAL_FORCE;
}

void CNPC_BlobFountain::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	#undef USE_BLOBULATOR // TODO (Ilya): Get this working
	#ifdef USE_BLOBULATOR
		m_force.AddForces( m_vecPhysParticles, m_nActiveParticles, m_flRadius, sv_lj_strength.GetFloat(), m_pBlobFountainController->m_vecLennardJonesForce );
	#endif
	//NetworkProp()->NetworkStateForceUpdate();
}

bool CNPC_BlobFountain::CreateVPhysics()
{
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);

	int nMaterialIndex = physprops->GetSurfaceIndex("water");

	// FIXME: don't hardcode the number of particles
	m_nActiveParticles = MAX_SURFACE_ELEMENTS;

	//bool result = BaseClass::CreateVPhysics( bFromRestore );

	m_pBlobFountainController = new CBlobFountainController( this );
	m_pMotionController = physenv->CreateMotionController( m_pBlobFountainController );

	// find the ground under the starting position
	trace_t	tr;
	UTIL_TraceLine( m_vecStart+Vector(0,0,1), m_vecStart-Vector(0,0,64), MASK_SOLID_BRUSHONLY | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP, this, COLLISION_GROUP_NONE, &tr );
	m_vecStart = tr.endpos;

	m_vecPhysParticles.EnsureCapacity( MAX_SURFACE_ELEMENTS );

	for (int i = 0; i < m_nActiveParticles; i++)
	{
		
		BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) = m_vecStart + Vector(0,0,10.0f);
		IPhysicsObject* p = physenv->CreateSphereObject( m_flRadius, nMaterialIndex, BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ), GetAbsAngles(), &params, false );
		if(p == NULL) continue;

		int index = m_vecPhysParticles.AddToTail(p);

		Vector vVelocity = Vector( RandomFloat( -1, 1 ), RandomFloat( -1, 1 ), RandomFloat( -1, 1 ) ) * 10.0f;
		p->SetVelocity( &vVelocity, NULL );
		//PhysSetGameFlags( m_vecPhysParticles[i], FVPHYSICS_MULTIOBJECT_ENTITY );
		PhysSetGameFlags( p, FVPHYSICS_NO_SELF_COLLISIONS | FVPHYSICS_MULTIOBJECT_ENTITY ); // call collisionruleschanged if this changes dynamically
		p->SetGameIndex( index );

		p->SetMass( 10.0f );
		p->EnableGravity( true );
		p->EnableDrag( true );

		// m_vecPhysParticles[i]->EnableMotion( false );

		float flDamping = 0.5f;
		float flAngDamping = 0.5f;
		p->SetDamping( &flDamping, &flAngDamping );
		//m_vecPhysParticles[i]->SetInertia( Vector( 1e30, 1e30, 1e30 ) );
		p->EnableGravity( true );
		//p->SetPosition(m_vecStart + Vector(0,0,10.0f), QAngle(0.0f, 0.0f, 0.0f), false);
		m_pMotionController->AttachObject( p, true );
		m_iContactTime[i] = 0;
		m_iMode[i] = 0;

		// 0: ready to launch, 1: launched, 2: contact
	}

	// If any physics objects failed to be created, this adjusts m_nActiveParticles
	m_nActiveParticles = m_vecPhysParticles.Count();

	return true;
}

void CNPC_BlobFountain::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	// the sphere hit is:  i = pEvent->pObjects[index]->GetGameIndex();
	// CBaseEntity *pHit = pEvent->pEntities[!index];
	int nSphere = pEvent->pObjects[index]->GetGameIndex();

	m_bContact[nSphere] = true;

#if 0
	static int count = 0;
	count ++;

	//if((float(rand()) / float(VALVE_RAND_MAX)) < 0.1f)
	if(count % 10 == 0)
	{
		// Create splash effect
		CEffectData	data;
		data.m_fFlags  = 0;

		Vector pos;
		pEvent->pObjects[index]->GetPosition( &pos, NULL );

		Vector fountainOrigin(-1980, -1792, 1);
		Vector rad = pos - fountainOrigin;
		//pos += (rad * 0.25f);
		data.m_vOrigin = pos + Vector(0, 0, 5.0f);
		// FIXME: needs to be the correct vector for the impact!
		rad.NormalizeInPlace();
		data.m_vNormal = rad;//Vector( 0, 0, 1 );
		data.m_flScale = 10.0f;
		DispatchEffect( "watersplash", data );			
	}
#endif
}


void CNPC_BlobFountain::RunAI( void )
{
	/*
	bool newPause = ((CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI) != 0);

	//(sv_lj_strength.GetFloat() > 0.0f);

	if(newPause != m_bPause)
	{
	if(newPause)
	{
	for (int i = 0; i < m_nActiveParticles; i++)
	{
	m_vecPhysParticles[i]->EnableMotion(false);
	}
	}
	else
	{
	for (int i = 0; i < m_nActiveParticles; i++)
	{
	m_vecPhysParticles[i]->EnableMotion(true);
	}
	}
	m_bPause = newPause;
	}

	if(m_bPause) return;
	*/

	m_pMotionController->WakeObjects();

	// push spheres around to meet position targets

	//Vector vecGoal = m_vecStart;
	for (int i = 0; i < m_nActiveParticles; i++)
	{
		m_vecPhysParticles[i]->EnableMotion(true);
		//m_vecPhysParticles[i]->EnableGravity( true );
		{
			Vector pos;
			m_vecPhysParticles[i]->GetPosition( &pos, NULL );
			BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) = pos;
			m_flSurfaceR[i] = m_fRadius[i];
		}
#if 0
		if (m_bContact[i])
		{
			Vector vecVel;
			m_vecPhysParticles[i]->GetVelocity( &vecVel, NULL );

			Vector estPos = m_vecSurfacePos[i] + vecVel;
			Vector delta( 0, 0, 0 );
			float dist(0.0f);

			// NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), m_bFloat[i] * 255, 255, m_bContact[i] * 255, 20, .1);
			// move sphere towards center target
			delta = vecGoal - estPos;
			dist = VectorNormalize( delta );
			// delta = delta * min( max( dist - m_flRadius * 4, 0 ), 500 ) * sv_surface_tension.GetFloat();
			delta = delta * min( dist, 100 ) * 0.2; // sv_surface_tension.GetFloat();

			/*
			if(dist > 10.0f)
			{
			m_vecPhysParticles[i]->SetPosition(m_vecStart + Vector(0,0,m_flRadius), QAngle(), true);
			Vector vVelocity = Vector( RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ) ) * 0.2f;
			m_vecPhysParticles[i]->SetVelocity( &vVelocity, NULL );
			}
			m_bContact[i] = false;
			*/

			/*
			if(dist < 10.0f)
			{
			m_vecPhysParticles[i]->ApplyForceCenter( Vector(0.0f, 0.0f, 10000.0f) );
			}
			*/

			/*
			m_flSurfaceV[i] = Approach( 0.0f, m_flSurfaceV[i], 0.2f );

			m_vecPhysParticles[i]->ApplyForceCenter( delta );
			*/


			//NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 255, 0, 20, .1);
		}
#endif
	}

	NetworkProp()->NetworkStateForceUpdate();
}

#else



IMotionEvent::simresult_e CBlobFountainController::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{

	int nSphere = pObject->GetGameIndex();

	if (m_pOwner && m_flLennardJonesTime != gpGlobals->curtime)
	{
		m_pOwner->Simulate( pController, pObject, deltaTime, linear, angular );
		m_flLennardJonesTime = gpGlobals->curtime;
	}

	linear += m_vecLennardJonesForce[nSphere] * (1.0f / deltaTime);

	Vector pos, vecVel;
	//QAngle ang;
	pObject->GetPosition( &pos, NULL );
	pObject->GetVelocity( &vecVel, NULL );
	//float d = sv_surface_radius.GetFloat();
	Vector nozzle = m_pOwner->GetNozzle();
	pos = pos - nozzle;
	float dist = pos.Length();
	pos.NormalizeInPlace();
	/*
	if(nSphere <= 200)
	{
	if(dist >= 150.0f)
	{
	linear -= pos * ((dist-150.0f) / deltaTime);
	}
	}
	else
	{
	*/
	if(dist >= 500.0f)
	{
		linear -= pos * ((dist-500.0f) / deltaTime);
	}
	//}

	float v = vecVel.Length();
	linear -=  vecVel * (1.0f / ((v+FLT_EPSILON) * deltaTime));


#if 0
	Vector pos, vecVel;
	QAngle ang;
	pObject->GetPosition( &pos, NULL );
	pObject->GetVelocity( &vecVel, NULL );
	float d = sv_surface_radius.GetFloat();
	Vector nozzle = m_pOwner->GetNozzle();
	Vector start = nozzle + Vector(0.0f,0.0f,d);
	pos = pos - nozzle;
	float dist = pos.Length();

	if(m_pOwner->m_iMode[nSphere] == 0)
	{
		if (dist < 10.0f)
		{
			if(vecVel.z < 0.1f && abs(vecVel.x) < 0.05f && abs(vecVel.y) < 0.05f)
			{
				m_pOwner->m_iContactTime[nSphere] = 0;
				m_pOwner->m_fRadius[nSphere] = 0.5f;
				m_pOwner->m_iMode[nSphere] = 1;
				m_pOwner->m_bContact[nSphere] = false;
				//linear.z += 10000 * (1.0f / deltaTime);
				//linear.z = 500000.0f;
				//linear = -vecVel / deltaTime;

				float v = 6000.0f / deltaTime;
				float azimuth = (2.0f * M_PI * (float(rand()) / float(VALVE_RAND_MAX)));
				float polar = 0.04f * M_PI + (0.01f * (float(rand()) / float(VALVE_RAND_MAX)));
				//static float azimuth = 0.0f;
				//azimuth += (0.05f * M_PI);
				//float cur_azimuth = azimuth + RandomFloat(-0.01, 0.01);

				linear.x = v * cos(azimuth) * sin(polar);
				linear.y = v * sin(azimuth) * sin(polar);
				linear.z = v * cos(polar);
			}
			//else
			//{
			//	pObject->SetVelocity( &Vector(0.0f, 0.0f, 0.0f), NULL );
			//}
		}
	}
	else if(m_pOwner->m_iMode[nSphere] == 1)
	{
		m_pOwner->m_fRadius[nSphere] = min(1.0f, m_pOwner->m_fRadius[nSphere] + 3.0f*deltaTime);
		if(m_pOwner->m_bContact[nSphere]) // vecVel.z < 0.1f && 
		{
			if(dist < 80.0f)
			{
				m_pOwner->m_fRadius[nSphere] = 0.0f;
				pObject->SetPosition(nozzle, QAngle(0.0f, 0.0f, 0.0f), true);
				Vector zero(0.0f, 0.0f, 0.0f);
				pObject->SetVelocity( &zero, NULL );
				m_pOwner->m_iMode[nSphere] = 0;
				m_pOwner->m_iContactTime[nSphere] = 0;
			}
			else
			{
				m_pOwner->m_fRadius[nSphere] = 0.0f; //max(0.0f, m_pOwner->m_fRadius[nSphere] - 2.0f*deltaTime);
				m_pOwner->m_iContactTime[nSphere] += deltaTime;
				if(m_pOwner->m_iContactTime[nSphere] >= 1.0f)
				{
					m_pOwner->m_iMode[nSphere] = 2;
					m_pOwner->m_iContactTime[nSphere] = 0;
					m_pOwner->m_bContact[nSphere] = false;
				}
			}
		}
	}
	else if(m_pOwner->m_iMode[nSphere] == 2)
	{
		pObject->SetPosition(nozzle, QAngle(0.0f, 0.0f, 0.0f), true);
		Vector zero(0.0f, 0.0f, 0.0f);
		pObject->SetVelocity( &zero, NULL );
		//Vector vVelocity = Vector( RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ) ) * 0.2f;
		//pObject->SetVelocity( &vVelocity, NULL );

		m_pOwner->m_iContactTime[nSphere] += deltaTime;
		if(m_pOwner->m_iContactTime[nSphere] >= 1.0f)
		{
			m_pOwner->m_iMode[nSphere] = 0;
			m_pOwner->m_iContactTime[nSphere] = 0;
		}
	}

#if 0
	if(m_pOwner->m_bContact[nSphere]) // || (vecVel.z < -20.0f && pos.z < 40.0f))
	{
		m_pOwner->m_flSurfaceR[nSphere] = 0.0f; //max(0.0f, m_pOwner->m_flSurfaceR[nSphere] - 0.1f);
		m_pOwner->m_iContactTime[nSphere] ++;

		//if(m_pOwner->m_flSurfaceR[nSphere] <= 0.0f)
		if(m_pOwner->m_iContactTime[nSphere] >= 10)
		{
			m_pOwner->m_bContact[nSphere] = false;

			if(dist > 10.0f)
			{
				pObject->SetPosition(nozzle, QAngle(0.0f, 0.0f, 0.0f), true);
				Vector vVelocity = Vector( RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ) ) * 0.2f;
				pObject->SetVelocity( &vVelocity, NULL );
				//m_pOwner->m_flSurfaceR[nSphere] = 0.1f


				//Vector vVelocity(0.0f, 0.0f, 0.0f);
				//pObject->SetVelocity( &vVelocity, NULL );
			}
		}
	}
	else
	{
		//pObject->SetVelocity( &Vector(), &AngularImpulse());
		if (dist < 10.0f)
		{
			if(vecVel.z < 0.1f && abs(vecVel.x) < 0.1f && abs(vecVel.y) < 0.1f)
			{
				m_pOwner->m_iContactTime[nSphere] = 0;
				m_pOwner->m_flSurfaceR[nSphere] = 1.0f;
				//linear.z += 10000 * (1.0f / deltaTime);
				//linear.z = 500000.0f;
				//linear = -vecVel / deltaTime;

				float v = 6000.0f / deltaTime;
				float azimuth = (2.0f * M_PI * (float(rand()) / float(VALVE_RAND_MAX)));
				float polar = 0.03f * M_PI; //(0.03f * M_PI * (float(rand()) / float(VALVE_RAND_MAX))); // 

				linear.x = v * cos(azimuth) * sin(polar);
				linear.y = v * sin(azimuth) * sin(polar);
				linear.z = v * cos(polar);
			}
		}
	}
#endif
#endif

	return IMotionEvent::SIM_GLOBAL_FORCE;
}

void CNPC_BlobFountain::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	m_force.AddForces( m_vecPhysParticles, m_nActiveParticles, m_flRadius * 3.0f, sv_lj_strength.GetFloat(), m_pBlobFountainController->m_vecLennardJonesForce );
	//NetworkProp()->NetworkStateForceUpdate();
}

bool CNPC_BlobFountain::CreateVPhysics( bool bFromRestore )
{
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);

	int nMaterialIndex = physprops->GetSurfaceIndex("water");

	// FIXME: don't hardcode the number of particles
	m_nActiveParticles = 300;

	//bool result = BaseClass::CreateVPhysics( bFromRestore );

	m_pBlobFountainController = new CBlobFountainController( this );
	m_pMotionController = physenv->CreateMotionController( m_pBlobFountainController );

	// find the ground under the starting position
	trace_t	tr;
	UTIL_TraceLine( m_vecStart+Vector(0,0,1), m_vecStart-Vector(0,0,64), MASK_SOLID_BRUSHONLY | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP, this, COLLISION_GROUP_NONE, &tr );
	m_vecStart = tr.endpos;
	m_vecStart.z += 600.0f;

	for (int i = 0; i < m_nActiveParticles; i++)
	{
		m_vecSurfacePos[i] = m_vecStart + Vector( RandomFloat( -1, 1 ), RandomFloat( -1, 1 ), RandomFloat( -1, 1 ) ) * 400.0f;;
		m_vecPhysParticles[i] = physenv->CreateSphereObject( m_flRadius, nMaterialIndex, m_vecSurfacePos[i], GetAbsAngles(), &params, false );

		if ( m_vecPhysParticles[i] )
		{
			Vector vVelocity = Vector( RandomFloat( -1, 1 ), RandomFloat( -1, 1 ), RandomFloat( -1, 1 ) ) * 0.0f;
			m_vecPhysParticles[i]->SetVelocity( &vVelocity, NULL );
			//PhysSetGameFlags( m_vecPhysParticles[i], FVPHYSICS_MULTIOBJECT_ENTITY );
			PhysSetGameFlags( m_vecPhysParticles[i], FVPHYSICS_NO_SELF_COLLISIONS | FVPHYSICS_MULTIOBJECT_ENTITY ); // call collisionruleschanged if this changes dynamically
			m_vecPhysParticles[i]->SetGameIndex( i );

			m_vecPhysParticles[i]->SetMass( 10.0f );
			m_vecPhysParticles[i]->EnableGravity( false );
			m_vecPhysParticles[i]->EnableDrag( true );
			float drag = 100.0f;
			m_vecPhysParticles[i]->SetDragCoefficient(&drag, &drag);

			m_vecPhysParticles[i]->EnableCollisions(false);

			// m_vecPhysParticles[i]->EnableMotion( false );

			float flDamping = 0.5f;
			float flAngDamping = 0.5f;
			m_vecPhysParticles[i]->SetDamping( &flDamping, &flAngDamping );
			//m_vecPhysParticles[i]->SetInertia( Vector( 1e30, 1e30, 1e30 ) );
			//m_vecPhysParticles[i]->EnableGravity( true );
			//m_vecPhysParticles[i]->SetPosition(m_vecStart + Vector(0,0,10.0f), QAngle(0.0f, 0.0f, 0.0f), false);
			m_pMotionController->AttachObject( m_vecPhysParticles[i], true );
			m_iContactTime[i] = 0;
			m_iMode[i] = 0;
			m_fRadius[i] = 1.0f;

			// 0: ready to launch, 1: launched, 2: contact
		}
	}

	return true;
}


void CNPC_BlobFountain::RunAI( void )
{
	m_pMotionController->WakeObjects();

	// push spheres around to meet position targets

	//Vector vecGoal = m_vecStart;
	for (int i = 0; i < m_nActiveParticles; i++)
	{
		//m_vecPhysParticles[i]->EnableGravity( true );
		{
			Vector pos;
			m_vecPhysParticles[i]->GetPosition( &pos, NULL );
			m_vecSurfacePos[i] = pos;
			m_flSurfaceR[i] = m_fRadius[i];
		}
#if 0
		if (m_bContact[i])
		{
			Vector vecVel;
			m_vecPhysParticles[i]->GetVelocity( &vecVel, NULL );

			Vector estPos = m_vecSurfacePos[i] + vecVel;
			Vector delta( 0, 0, 0 );
			float dist(0.0f);

			// NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), m_bFloat[i] * 255, 255, m_bContact[i] * 255, 20, .1);
			// move sphere towards center target
			delta = vecGoal - estPos;
			dist = VectorNormalize( delta );
			// delta = delta * min( max( dist - m_flRadius * 4, 0 ), 500 ) * sv_surface_tension.GetFloat();
			delta = delta * min( dist, 100 ) * 0.2; // sv_surface_tension.GetFloat();

			/*
			if(dist > 10.0f)
			{
			m_vecPhysParticles[i]->SetPosition(m_vecStart + Vector(0,0,m_flRadius), QAngle(), true);
			Vector vVelocity = Vector( RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ), RandomFloat( -1.0f, 1.0f ) ) * 0.2f;
			m_vecPhysParticles[i]->SetVelocity( &vVelocity, NULL );
			}
			m_bContact[i] = false;
			*/

			/*
			if(dist < 10.0f)
			{
			m_vecPhysParticles[i]->ApplyForceCenter( Vector(0.0f, 0.0f, 10000.0f) );
			}
			*/

			/*
			m_flSurfaceV[i] = Approach( 0.0f, m_flSurfaceV[i], 0.2f );

			m_vecPhysParticles[i]->ApplyForceCenter( delta );
			*/


			//NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 255, 0, 20, .1);
		}
#endif
	}

	NetworkProp()->NetworkStateForceUpdate();
}

#endif
