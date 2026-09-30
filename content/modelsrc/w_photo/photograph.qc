// Created by Crowbar 0.74

$modelname "weapons/w_photo.mdl"

$staticprop

$bodygroup "Body"
{
	studio "photograph.smd"
}


$surfaceprop "metal"

$contents "solid"

$illumposition 0 -0.011 0

$cdmaterials "models\weapons\v_photo\"

$cbox 0 0 0 0 0 0

$bbox -0.532 -11.25 -8.25 0.555 11.25 8.25

$definebone "static_prop" "" 0 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"photograph_anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$collisionmodel "photograph_physics.smd"
{
	$mass 2
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 1

}
