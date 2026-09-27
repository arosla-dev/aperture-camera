// Created by Crowbar 0.74

$modelname "weapons/v_photo.mdl"

$bodygroup "studio"
{
	studio "v_photo_reference.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition -2.543 -20.788 -20.539

$cdmaterials "models\weapons\"
$cdmaterials "models\weapons\v_photo\"

$attachment "controlpanel0_ur" "ValveBiped.Camera" -3 1.2 1.2 rotate 0 180 90
$attachment "controlpanel0_ll" "ValveBiped.Camera" 2.8 1.2 -2.2 rotate 0 180 90

$cbox 0 0 0 0 0 0

$bbox 7.504 -17.76 -40.406 34.071 12.673 -0.672

$definebone "ValveBiped.Bip01" "" 0 -0.533595 38.566879 0 0 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine" "ValveBiped.Bip01" 0 3.345131 -2.981898 4.76655 89.999927 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine1" "ValveBiped.Bip01_Spine" 3.995834 -0.000004 -0.000003 -0.000057 -1.651208 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine2" "ValveBiped.Bip01_Spine1" 3.520294 0.000005 -0.000006 0 5.575509 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine4" "ValveBiped.Bip01_Spine2" 8.921486 -0.000023 -0.000014 0.000057 10.68022 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Clavicle" "ValveBiped.Bip01_Spine4" 2.059525 0.97793 1.937663 -73.711975 169.031582 91.085087 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_UpperArm" "ValveBiped.Bip01_L_Clavicle" 6.028159 -0.000069 0.000009 2.956747 -33.511674 -90.175074 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Forearm" "ValveBiped.Bip01_L_UpperArm" 11.692566 0.000046 0.000011 -0.000002 -3.45929 -0.000114 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Hand" "ValveBiped.Bip01_L_Forearm" 11.481682 0.000088 -0.000031 -2.21792 2.064137 89.856514 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger4" "ValveBiped.Bip01_L_Hand" 3.850407 0.126099 -1.223457 6.419817 -20.368536 -20.848901 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger41" "ValveBiped.Bip01_L_Finger4" 1.312565 -0.000008 0 3.117117 -23.434657 -0.000113 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger42" "ValveBiped.Bip01_L_Finger41" 0.729366 0.000002 -0.000002 2.33503 -13.807704 -0.000053 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger3" "ValveBiped.Bip01_L_Hand" 3.92634 0.316486 -0.46253 2.416849 -15.451696 -5.815465 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger31" "ValveBiped.Bip01_L_Finger3" 1.539091 0.000008 -0.000002 0.214113 -14.198581 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger32" "ValveBiped.Bip01_L_Finger31" 1.196321 0 -0.000002 0.353399 -24.684049 -0.000058 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger2" "ValveBiped.Bip01_L_Hand" 3.881376 0.214695 0.400243 -0.654261 -7.5941 5.162635 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger21" "ValveBiped.Bip01_L_Finger2" 1.719578 0.000023 -0.000021 -1.170153 -20.968191 -0.000287 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger22" "ValveBiped.Bip01_L_Finger21" 1.209181 -0.000004 -0.000001 -0.65472 -12.382528 0.000056 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger1" "ValveBiped.Bip01_L_Hand" 3.871292 0.117416 1.300888 -2.372332 -7.207867 18.371433 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger11" "ValveBiped.Bip01_L_Finger1" 1.719431 -0.000023 0.000019 -6.086358 -19.928211 0.000057 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger12" "ValveBiped.Bip01_L_Finger11" 1.099676 0 0.000006 -3.893532 -13.458196 0.000228 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger0" "ValveBiped.Bip01_L_Hand" 0.838837 -0.300282 1.312771 -31.613975 -42.436348 -71.229891 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger01" "ValveBiped.Bip01_L_Finger0" 1.789787 0.000006 0.000006 -0.212681 13.098045 -0.000116 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger02" "ValveBiped.Bip01_L_Finger01" 1.207012 0.000011 0 -0.324693 20.797965 -0.000058 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Clavicle" "ValveBiped.Bip01_Spine4" 2.059525 0.977934 -1.937662 73.711859 169.031131 -95.201041 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_UpperArm" "ValveBiped.Bip01_R_Clavicle" 6.028144 -0.000011 0.000002 -0.680904 -33.62053 93.605837 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Forearm" "ValveBiped.Bip01_R_UpperArm" 11.692577 0.000001 0.000004 0 -3.459289 0.000402 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped.Bip01_R_Forearm" 11.481657 0.000043 0.000019 2.217346 2.064253 -89.856739 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger4" "ValveBiped.Bip01_R_Hand" 3.850428 0.126122 1.223433 -6.41942 -20.368077 20.848731 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger41" "ValveBiped.Bip01_R_Finger4" 1.312567 0 0 -3.117065 -23.434669 0.000169 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger42" "ValveBiped.Bip01_R_Finger41" 0.729362 0.000004 0.000003 -2.334977 -13.807668 0.000173 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger3" "ValveBiped.Bip01_R_Hand" 3.926355 0.316505 0.462507 -2.416451 -15.451186 5.815406 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger31" "ValveBiped.Bip01_R_Finger3" 1.539089 0.000008 0.000003 -0.213944 -14.198583 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger32" "ValveBiped.Bip01_R_Finger31" 1.196316 0.000004 0.000005 -0.353402 -24.684119 0.000115 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger2" "ValveBiped.Bip01_R_Hand" 3.881388 0.21471 -0.400268 0.654431 -7.593349 -5.162694 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger21" "ValveBiped.Bip01_R_Finger2" 1.719567 0.000023 0.000016 1.170437 -20.968481 0.000287 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger22" "ValveBiped.Bip01_R_Finger21" 1.209175 0.000008 0.000005 0.654718 -12.382598 0.000058 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger1" "ValveBiped.Bip01_R_Hand" 3.871305 0.117447 -1.300912 2.372847 -7.207976 -18.371202 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger11" "ValveBiped.Bip01_R_Finger1" 1.719419 -0.000008 -0.000008 6.086015 -19.927815 -0.000226 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger12" "ValveBiped.Bip01_R_Finger11" 1.099667 -0.000004 -0.000001 3.89365 -13.458153 -0.000228 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger0" "ValveBiped.Bip01_R_Hand" 0.838846 -0.300285 -1.312777 31.614261 -42.435723 71.230287 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger01" "ValveBiped.Bip01_R_Finger0" 1.789783 -0.000002 -0.000002 0.212623 13.098039 -0.000517 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger02" "ValveBiped.Bip01_R_Finger01" 1.207001 0.000009 0 0.324692 20.797965 -0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Photograph" "ValveBiped.Bip01_R_Hand" -1.754572 -6.090916 0.038778 -0.943604 -132.423612 89.105767 0 0 0 0 0 0
$definebone "ValveBiped.paper1" "ValveBiped.Photograph" 0 0.196712 0 0.000009 0.000057 90.000037 0 0 0 0 0 0
$definebone "ValveBiped.paper2" "ValveBiped.paper1" 0.000002 2.032589 0 0 -0.000009 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.paper3" "ValveBiped.paper2" 0.000002 1.571991 0 0 -0.000009 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Camera" "ValveBiped.Bip01_R_Hand" 3.775905 -5.485672 -0.527956 -7.633229 -94.073138 -128.817641 0 0 0 0 0 0


$sectionframes 30 160

$sequence "idle" {
	"v_photo_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
	loop
}

$sequence "photo" {
	"v_photo_anims\photo.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 1
}

$sequence "draw" {
	"v_photo_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$sequence "holster" {
	"v_photo_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}
