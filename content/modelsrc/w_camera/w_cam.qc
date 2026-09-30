// Created by Crowbar 0.74

$modelname "weapons\w_cam.mdl"

$staticprop

$bodygroup "Body"
{
	studio "w_camera_reference.smd"
}


$surfaceprop "metal"

$contents "solid"

$illumposition 0.026 -2.051 0.305

$cdmaterials "models\weapons\v_cam\"

$cbox 0 0 0 0 0 0

$bbox -1.896 -7.75 -4.25 6.25 7.875 4.61

$definebone "static_prop" "" 0 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"w_cam_anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$collisionmodel "w_cam_physics.smd"
{
	$mass 2
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 3

}
