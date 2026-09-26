//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PORTAL_PLACEMENT_H
#define PORTAL_PLACEMENT_H
#ifdef _WIN32
#pragma once
#endif


struct CPortalCornerFitData;

bool FitPortalOnSurface(const CProp_Portal* pIgnorePortal, Vector& vOrigin, const Vector& vForward, const Vector& vRight,
	const Vector& vTopEdge, const Vector& vBottomEdge, const Vector& vRightEdge, const Vector& vLeftEdge,
	int iPlacedBy, ITraceFilter* pTraceFilterPortalShot,
	float fHalfWidth, float fHalfHeight,
	int iRecursions = 0, const CPortalCornerFitData* pPortalCornerFitData = 0, const int* p_piIntersectionIndex = 0, const int* piIntersectionCount = 0);
bool IsPortalIntersectingNoPortalVolume(const Vector& vOrigin, const QAngle& qAngles, const Vector& vForward, float fHalfWidth, float fHalfHeight);
bool IsPortalOverlappingOtherPortals(const CProp_Portal* pIgnorePortal, const Vector& vOrigin, const QAngle& qAngles, float fHalfWidth, float fHalfHeight, bool bFizzle = false);
float VerifyPortalPlacement(const CProp_Portal* pIgnorePortal, Vector& vOrigin, QAngle& qAngles, float fHalfWidth, float fHalfHeight, int iPlacedBy, bool bTest = false);


#endif // PORTAL_PLACEMENT_H
