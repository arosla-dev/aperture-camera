//==== Copyright © 2025-2026, Krillex, All rights reserved. ====//
//
// Purpose: F-Stop Camera viewfinder HUD overlay
//
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "view_scene.h"

#include "portal/weapon_camera_shared.h"
#include "portal/weapon_placement_shared.h"

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

class CHudItemSelect : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudItemSelect, vgui::Panel);

public:
	CHudItemSelect(const char *pElementName);

	virtual void Init();
	virtual void OnThink() override;

	// Icons active sizes
	const Vector2D m_cIconNeutral_SizeActive = Vector2D(55.0f, 55.0f);
	const Vector2D m_cIconCamera_SizeActive = Vector2D(55.0f, 55.0f);
	const Vector2D m_cIconPicture_SizeActive = Vector2D(55.0f, 55.0f);
	// Icons inactive sizes
	const Vector2D m_cIconNeutral_SizeInactive = Vector2D(45.0f, 45.0f);
	const Vector2D m_cIconCamera_SizeInactive = Vector2D(45.0f, 45.0f);
	const Vector2D m_cIconPicture_SizeInactive = Vector2D(45.0f, 45.0f);
	// Icons offsets
	const Vector2D m_cIconNeutral_Offset = Vector2D(0.0f, 50.0f);
	const Vector2D m_cIconCamera_OffsetInactive = Vector2D(70.0f, 50.0f);
	const Vector2D m_cIconPicture_OffsetInactive = Vector2D(-70.0f, 50.0f);
	const Vector2D m_cIconCamera_OffsetActive = Vector2D(75.0f, 50.0f);
	const Vector2D m_cIconPicture_OffsetActive = Vector2D(-75.0f, 50.0f);
	// Icons inactive colors
	const float m_flIconInactive_Brightness = 125.0f;
	const float m_flIconActive_Brightness = 255.0f;

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint();
	virtual bool ShouldDraw();

private:
	// Textures
	int m_IconNeutral;
	int m_IconCamera;
	int m_IconPicture;

	inline bool IsCameraActive();
	inline bool IsPlacementActive();

	void UpdateNeutralIcon();
	void UpdateCameraIcon();
	void UpdatePlacementIcon();

	bool bWasCameraActive = false;
	bool bWasPlacementActive = false;
	bool bWasNeutralActive = true;

	// Icons sizes
	Vector2D m_cIconNeutral_Size = m_cIconNeutral_SizeInactive;
	Vector2D m_cIconCamera_Size = m_cIconCamera_SizeInactive;
	Vector2D m_cIconPicture_Size = m_cIconPicture_SizeInactive;
	// Icons offsets
	Vector2D m_cIconCamera_Offset = m_cIconCamera_OffsetInactive;
	Vector2D m_cIconPicture_Offset = m_cIconPicture_OffsetInactive;
	// Icons colors
	float m_flIconNeutral_Brightness = m_flIconActive_Brightness;
	float m_flIconCamera_Brightness = m_flIconInactive_Brightness;
	float m_flIconPicture_Brightness = m_flIconInactive_Brightness;
	// Target icons sizes
	Vector2D m_cIconNeutral_TargetSize = m_cIconNeutral_SizeInactive;
	Vector2D m_cIconCamera_TargetSize = m_cIconCamera_SizeInactive;
	Vector2D m_cIconPicture_TargetSize = m_cIconPicture_SizeInactive;
	// Target icons offsets
	Vector2D m_cIconCamera_TargetOffset = m_cIconCamera_OffsetInactive;
	Vector2D m_cIconPicture_TargetOffset = m_cIconPicture_OffsetInactive;
	// Target icons colors
	float m_flIconNeutral_TargetBrightness = m_flIconActive_Brightness;
	float m_flIconCamera_TargetBrightness = m_flIconInactive_Brightness;
	float m_flIconPicture_TargetBrightness = m_flIconInactive_Brightness;
	// Lerp times
	float m_flIconNeutral_LerpTime = 1.0f;
	float m_flIconCamera_LerpTime = 1.0f;
	float m_flIconPicture_LerpTime = 1.0f;
};

DECLARE_HUDELEMENT_DEPTH(CHudItemSelect, 100);

CHudItemSelect::CHudItemSelect(const char *pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudViewfinder")
{
	vgui::Panel *pParent = GetClientMode()->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

void CHudItemSelect::Init()
{
	m_IconNeutral = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_IconNeutral, "HUD/hud_icon_neutral", true, false);

	m_IconCamera = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_IconCamera, "HUD/hud_icon_camera", true, false);

	m_IconPicture = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_IconPicture, "HUD/hud_icon_picture", true, false);

	UnregisterForRenderGroup("global");
}

void CHudItemSelect::OnThink() {
	BaseClass::OnThink();
	// Neutral icon smooth animation
	UpdateNeutralIcon();
	if (m_cIconNeutral_Size != m_cIconNeutral_TargetSize || m_flIconNeutral_Brightness != m_flIconNeutral_TargetBrightness) {
		m_cIconNeutral_Size = Lerp(m_flIconNeutral_LerpTime, m_cIconNeutral_Size, m_cIconNeutral_TargetSize);
		m_flIconNeutral_Brightness = Lerp(m_flIconNeutral_LerpTime, m_flIconNeutral_Brightness, m_flIconNeutral_TargetBrightness);
		m_flIconNeutral_LerpTime = clamp(m_flIconNeutral_LerpTime + 0.010f, 0.f, 1.f);
	}
	// Camera icon smooth animation
	UpdateCameraIcon();
	if (m_cIconCamera_Size != m_cIconCamera_TargetSize || m_cIconCamera_Offset != m_cIconCamera_TargetOffset || m_flIconCamera_Brightness != m_flIconCamera_TargetBrightness) {
		m_cIconCamera_Size = Lerp(m_flIconCamera_LerpTime, m_cIconCamera_Size, m_cIconCamera_TargetSize);
		m_cIconCamera_Offset = Lerp(m_flIconCamera_LerpTime, m_cIconCamera_Offset, m_cIconCamera_TargetOffset);
		m_flIconCamera_Brightness = Lerp(m_flIconCamera_LerpTime, m_flIconCamera_Brightness, m_flIconCamera_TargetBrightness);
		m_flIconCamera_LerpTime = clamp(m_flIconCamera_LerpTime + 0.025f, 0.f, 1.f);
	}
	// Picture icon smooth animation
	UpdatePlacementIcon();
	if (m_cIconPicture_Size != m_cIconPicture_TargetSize || m_cIconPicture_Offset != m_cIconPicture_TargetOffset || m_flIconPicture_Brightness != m_flIconPicture_TargetBrightness) {
		m_cIconPicture_Size = Lerp(m_flIconPicture_LerpTime, m_cIconPicture_Size, m_cIconPicture_TargetSize);
		m_cIconPicture_Offset = Lerp(m_flIconPicture_LerpTime, m_cIconPicture_Offset, m_cIconPicture_TargetOffset);
		m_flIconPicture_Brightness = Lerp(m_flIconPicture_LerpTime, m_flIconPicture_Brightness, m_flIconPicture_TargetBrightness);
		m_flIconPicture_LerpTime = clamp(m_flIconPicture_LerpTime + 0.025f, 0.f, 1.f);
	}
}

void CHudItemSelect::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	// Transparency
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	int width, height;
	GetHudSize(width, height);
	SetBounds(0, 0, width, height);
}

void CHudItemSelect::Paint()
{
	int screenWidth, screenHeight;
	GetHudSize(screenWidth, screenHeight);

	float xWide = screenWidth;
	float xMid = xWide / 2;

	// Draw icon neutral
	vgui::surface()->DrawSetTexture(m_IconNeutral);
	vgui::surface()->DrawSetColor(m_flIconNeutral_Brightness, m_flIconNeutral_Brightness, m_flIconNeutral_Brightness, 255);
	vgui::surface()->DrawTexturedRect(xMid - m_cIconNeutral_Size.x - m_cIconNeutral_Offset.x, m_cIconNeutral_Offset.y - m_cIconNeutral_Size.y, xMid + m_cIconNeutral_Size.x - m_cIconNeutral_Offset.x, m_cIconNeutral_Offset.y + m_cIconNeutral_Size.y);

	// Draw icon camera
	vgui::surface()->DrawSetTexture(m_IconCamera);
	vgui::surface()->DrawSetColor(m_flIconCamera_Brightness, m_flIconCamera_Brightness, m_flIconCamera_Brightness, 255);
	vgui::surface()->DrawTexturedRect(xMid - m_cIconCamera_Size.x - m_cIconCamera_Offset.x, m_cIconCamera_Offset.y - m_cIconCamera_Size.y, xMid + m_cIconCamera_Size.x - m_cIconCamera_Offset.x, m_cIconCamera_Offset.y + m_cIconCamera_Size.y);
	
	// Draw icon picture
	vgui::surface()->DrawSetTexture(m_IconPicture);
	vgui::surface()->DrawSetColor(m_flIconPicture_Brightness, m_flIconPicture_Brightness, m_flIconPicture_Brightness, 255);
	vgui::surface()->DrawTexturedRect(xMid - m_cIconPicture_Size.x - m_cIconPicture_Offset.x, m_cIconPicture_Offset.y - m_cIconPicture_Size.y, xMid + m_cIconPicture_Size.x - m_cIconPicture_Offset.x, m_cIconPicture_Offset.y + m_cIconPicture_Size.y);
}

bool CHudItemSelect::ShouldDraw()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	C_WeaponCamera *pCamera = dynamic_cast<C_WeaponCamera *>(pPlayer->GetActiveWeapon());
	C_WeaponPlacement *pPlacement = dynamic_cast<C_WeaponPlacement *>(pPlayer->GetActiveWeapon());

	if (!pCamera) { if (!pPlacement) { return false; } }

	return CHudElement::ShouldDraw();
}

inline bool CHudItemSelect::IsCameraActive()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;
	C_WeaponCamera *pCamera = dynamic_cast<C_WeaponCamera *>(pPlayer->GetActiveWeapon());
	if (pCamera) { return true; }
	else { return false;  }
}

inline bool CHudItemSelect::IsPlacementActive()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;
	C_WeaponPlacement *pPlacement = dynamic_cast<C_WeaponPlacement *>(pPlayer->GetActiveWeapon());
	if (pPlacement) { return true; }
	else { return false; }
}

void CHudItemSelect::UpdateNeutralIcon() {
	if (IsCameraActive() == false && IsPlacementActive() == false) { 
		m_cIconNeutral_TargetSize = m_cIconNeutral_SizeActive; 
		m_flIconNeutral_TargetBrightness = m_flIconActive_Brightness;

		if (bWasNeutralActive == false) {
			m_flIconNeutral_LerpTime = 0.0f;
			bWasNeutralActive = true;
		}
	}
	else { 
		m_cIconNeutral_TargetSize = m_cIconNeutral_SizeInactive;
		m_flIconNeutral_TargetBrightness = m_flIconInactive_Brightness;
		if (bWasNeutralActive == true) {
			m_flIconNeutral_LerpTime = 0.0f;
			bWasNeutralActive = false;
		}
	}
}

void CHudItemSelect::UpdateCameraIcon() {
	if (IsCameraActive() == true) {
		m_cIconCamera_TargetSize = m_cIconCamera_SizeActive;
		m_cIconCamera_TargetOffset = m_cIconCamera_OffsetActive;
		m_flIconCamera_TargetBrightness = m_flIconActive_Brightness;

		if (bWasCameraActive == false) {
			m_flIconCamera_LerpTime = 0.0f;
			bWasCameraActive = true;
		}	
	}
	else {
		m_cIconCamera_TargetSize = m_cIconCamera_SizeInactive;
		m_cIconCamera_TargetOffset = m_cIconCamera_OffsetInactive;
		m_flIconCamera_TargetBrightness = m_flIconInactive_Brightness;;

		if (bWasCameraActive == true) {
			m_flIconCamera_LerpTime = 0.0f;
			bWasCameraActive = false;
		}
	}
}

void CHudItemSelect::UpdatePlacementIcon() {
	if (IsPlacementActive() == true) {
		m_cIconPicture_TargetSize = m_cIconPicture_SizeActive;
		m_cIconPicture_TargetOffset = m_cIconPicture_OffsetActive;
		m_flIconPicture_TargetBrightness = m_flIconActive_Brightness;

		if (bWasPlacementActive == false) {
			m_flIconPicture_LerpTime = 0.0f;
			bWasPlacementActive = true;
		}
	}
	else {
		m_cIconPicture_TargetSize = m_cIconPicture_SizeInactive;
		m_cIconPicture_TargetOffset = m_cIconPicture_OffsetInactive;
		m_flIconPicture_TargetBrightness = m_flIconInactive_Brightness;

		if (bWasPlacementActive == true) {
			m_flIconPicture_LerpTime = 0.0f;
			bWasPlacementActive = false;
		}
	}
}