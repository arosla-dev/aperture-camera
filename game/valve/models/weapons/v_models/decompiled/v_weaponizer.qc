// Created by Crowbar 0.74

$modelname "weapons\v_models\v_weaponizer.mdl"

$bodygroup "v_weaponizer"
{
	studio "v_weaponizer_ref.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition -10.067 -24.746 -7.823

$cdmaterials "models\weapons\v_model\v_weaponizer\"
$cdmaterials ""

$attachment "ltube_muzzle" "v_weaponizer_handle" 35.91 -0.29 -5.24 rotate 0 0 0
$attachment "rtube_muzzle" "v_weaponizer_handle" 35.91 4.12 -1.99 rotate 0 0 0
$attachment "rtube_pattach_base" "v_weaponizer_handle" 18.48 4.13 -1.96 rotate 90 0 0
$attachment "ltube_pattach_base" "v_weaponizer_handle" 18.48 -0.3 -5.25 rotate 90 0 0
$attachment "rtube_pattach_end" "v_weaponizer_handle" 33.83 4.13 -1.96 rotate 90 0 0
$attachment "ltube_pattach_end" "v_weaponizer_handle" 33.83 -0.3 -5.25 rotate 90 0 0
$attachment "buff_attach" "v_weaponizer_handle" 10 4 4 rotate 90 0 0

$cbox 0 0 0 0 0 0

$bbox -12.977 -20.135 -17.544 62.47 0 1.897

$definebone "v_weaponizer_root" "" 0 0 0 90.000003 0 0 0 0 0 0 0 0
$definebone "v_weaponizer_handle" "v_weaponizer_root" 12.977451 1.897321 0 0 -180.000005 -180.000005 0 0 0 0 0 0


// The 32 below is a guess until further is known about the format.
$animblocksize 32 nostall

$bonesaveframe "v_weaponizer_root" position rotation
$bonesaveframe "v_weaponizer_handle" position rotation

$sequence "idle" {
	"v_weaponizer_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
	loop
}

$sequence "draw" {
	"v_weaponizer_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	{ event 5004 1 "" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "dryfire" {
	"v_weaponizer_anims\dryfire.smd"
	activity "ACT_VM_DRYFIRE" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "discharge" {
	"v_weaponizer_anims\discharge.smd"
	activity "ACT_VM_PULLBACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "attack_loop" {
	"v_weaponizer_anims\attack_loop.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5004 11 "" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "attack_kick" {
	"v_weaponizer_anims\attack_kick.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event 5004 11 "" }
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "absorb" {
	"v_weaponizer_anims\absorb.smd"
	activity "ACT_VM_RELOAD" 1
	{ event 5004 11 "" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}
