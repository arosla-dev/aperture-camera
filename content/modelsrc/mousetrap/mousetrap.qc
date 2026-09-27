// Created by Crowbar 0.74

$modelname "props_gameplay\mousetrap.mdl"

$bodygroup "Body"
{
	studio "mousetrap001_reference.smd"
}


$surfaceprop "wood"

$contents "solid"

$illumposition 0.058 -0.01 0.962

$cdmaterials "models\props_gameplay"
$cdmaterials ""

$cbox 0 0 0 0 0 0

$bbox -7.564 -3.716 0 7.584 3.831 1.924

$definebone "moustrap_base" "" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "bracket1" "moustrap_base" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "spring" "moustrap_base" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "snapper" "moustrap_base" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "snapper_lip" "snapper" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "snapper_pin" "moustrap_base" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "bracket4" "moustrap_base" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "bracket5" "moustrap_base" 0 0 0 0 0 0 0 0 0 0 0 0
$definebone "cheeze" "moustrap_base" 0 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"mousetrap_anims\idle.smd"
	activity "ACT_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "snap" {
	"mousetrap_anims\snap.smd"
	activity "ACT_USE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$collisionmodel "mousetrap_physics.smd"
{
	$mass 10
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 3

}
