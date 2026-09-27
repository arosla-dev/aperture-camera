// Created by Crowbar 0.74

$modelname "weapons/v_cam_expo.mdl"

$bodygroup "studio"
{
	studio "v_cam_reference.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition 0.103 -19.462 -16.11

$cdmaterials "models\weapons\"
$cdmaterials "models\weapons\v_cam\"

$attachment "controlpanel0_ur" "ValveBiped.Camera" -3 1.2 1.2 rotate 0 180 90
$attachment "controlpanel0_ll" "ValveBiped.Camera" 2.8 1.2 -2.2 rotate 0 180 90

$cbox 0 0 0 0 0 0

$bbox 8.722 -14.552 -29.422 30.202 14.758 -2.798

$definebone "ValveBiped.Bip01" "" 0 -0.533595 38.566879 0 0 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine" "ValveBiped.Bip01" 0 3.345131 -2.981898 4.76655 89.999927 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine1" "ValveBiped.Bip01_Spine" 3.995834 -0.000004 -0.000003 0 -1.651322 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine2" "ValveBiped.Bip01_Spine1" 3.520294 0.000014 -0.000001 0 5.575624 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine4" "ValveBiped.Bip01_Spine2" 8.921478 -0.000016 -0.000001 0 10.680505 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Clavicle" "ValveBiped.Bip01_Spine4" 2.059521 0.977922 1.937662 -73.711921 169.031186 91.085258 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_UpperArm" "ValveBiped.Bip01_L_Clavicle" 6.028151 -0.000031 0.000004 2.956978 -33.511609 -90.17519 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Forearm" "ValveBiped.Bip01_L_UpperArm" 11.692549 0.000005 0.000011 -0.000009 -3.45929 -0.000115 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Hand" "ValveBiped.Bip01_L_Forearm" 11.48167 0.000049 -0.000031 -2.217929 2.064023 89.856569 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger4" "ValveBiped.Bip01_L_Hand" 3.850407 0.126099 -1.223449 6.419763 -20.368476 -20.848905 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger41" "ValveBiped.Bip01_L_Finger4" 1.312561 -0.000008 0 3.117065 -23.434784 0.000009 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger42" "ValveBiped.Bip01_L_Finger41" 0.729362 0.000004 -0.000003 2.334979 -13.807721 -0.000108 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger3" "ValveBiped.Bip01_L_Hand" 3.92634 0.31649 -0.462523 2.416736 -15.451633 -5.815463 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger31" "ValveBiped.Bip01_L_Finger3" 1.539095 0.000004 -0.000002 0.214 -14.198689 0.000004 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger32" "ValveBiped.Bip01_L_Finger31" 1.196325 0 -0.000004 0.353459 -24.684054 -0.000055 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger2" "ValveBiped.Bip01_L_Hand" 3.881374 0.214699 0.40025 -0.654203 -7.593859 5.162695 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger21" "ValveBiped.Bip01_L_Finger2" 1.71958 0.000011 -0.000015 -1.170266 -20.968247 -0.000231 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger22" "ValveBiped.Bip01_L_Finger21" 1.209179 -0.000004 0 -0.654891 -12.382766 -0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger1" "ValveBiped.Bip01_L_Hand" 3.87129 0.11742 1.300894 -2.372675 -7.20832 18.371317 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger11" "ValveBiped.Bip01_L_Finger1" 1.719426 -0.000004 0.000007 -6.086073 -19.927931 0.000109 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger12" "ValveBiped.Bip01_L_Finger11" 1.099669 -0.000004 0 -3.893594 -13.458211 0.000108 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger0" "ValveBiped.Bip01_L_Hand" 0.838832 -0.300278 1.312772 -31.614036 -42.436351 -71.22983 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger01" "ValveBiped.Bip01_L_Finger0" 1.789783 0.000002 0.000006 -0.212682 13.098041 -0.000111 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger02" "ValveBiped.Bip01_L_Finger01" 1.207008 0.000009 -0.000002 -0.324696 20.797962 0.000004 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Clavicle" "ValveBiped.Bip01_Spine4" 2.059521 0.977923 -1.937661 73.711921 169.030953 -95.200987 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_UpperArm" "ValveBiped.Bip01_R_Clavicle" 6.02814 -0.000004 0.000001 -0.680789 -33.620524 93.606008 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Forearm" "ValveBiped.Bip01_R_UpperArm" 11.69256 0.000021 -0.000004 0.000012 -3.459289 0.000401 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped.Bip01_R_Forearm" 11.481653 0.000064 0.000015 2.217645 2.06431 -89.856855 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger4" "ValveBiped.Bip01_R_Hand" 3.85043 0.126106 1.223434 -6.419478 -20.368292 20.848789 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger41" "ValveBiped.Bip01_R_Finger4" 1.312565 0 -0.000002 -3.117061 -23.434654 0.000051 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger42" "ValveBiped.Bip01_R_Finger41" 0.729366 0.000004 0.000003 -2.334976 -13.80771 0.000108 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger3" "ValveBiped.Bip01_R_Hand" 3.926358 0.31649 0.462509 -2.416507 -15.451461 5.815407 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger31" "ValveBiped.Bip01_R_Finger3" 1.539099 0.000004 0.000002 -0.214 -14.198517 -0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger32" "ValveBiped.Bip01_R_Finger31" 1.19632 0.000008 0.000004 -0.353401 -24.684116 0.000113 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger2" "ValveBiped.Bip01_R_Hand" 3.881389 0.214699 -0.400265 0.654375 -7.593566 -5.162636 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger21" "ValveBiped.Bip01_R_Finger2" 1.719571 0.000019 0.000015 1.170379 -20.968411 0.000172 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger22" "ValveBiped.Bip01_R_Finger21" 1.209175 0.000004 0.000003 0.654775 -12.382639 0.000058 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger1" "ValveBiped.Bip01_R_Hand" 3.871308 0.117424 -1.300909 2.372847 -7.208197 -18.371319 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger11" "ValveBiped.Bip01_R_Finger1" 1.719421 -0.000008 -0.000008 6.08607 -19.927809 -0.000114 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger12" "ValveBiped.Bip01_R_Finger11" 1.099667 -0.000004 0 3.893589 -13.458145 -0.000115 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger0" "ValveBiped.Bip01_R_Hand" 0.838847 -0.300285 -1.312776 31.61421 -42.435996 71.230226 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger01" "ValveBiped.Bip01_R_Finger0" 1.789776 0 0.000002 0.212636 13.098044 -0.000461 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger02" "ValveBiped.Bip01_R_Finger01" 1.207008 0.000007 0 0.324705 20.797859 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Camera" "ValveBiped.Bip01_R_Hand" -1.754601 -6.090904 0.038776 -0.943604 -132.423899 89.105821 0 0 0 0 0 0


$sectionframes 30 121

$sequence "idle" {
	"v_cam_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
	loop
}

$sequence "photo" {
	"v_cam_anims\photo.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "draw" {
	"v_cam_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$sequence "holster" {
	"v_cam_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$sequence "admire" {
	"v_cam_anims\admire.smd"
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$sequence "no" {
	"v_cam_anims\no.smd"
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
}
