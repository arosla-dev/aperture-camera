//==== Blobulator. Copyright © 2005-2006, Rosware LLC, All rights reserved. ====//

#include "physparticlecache.h"


/*
void PhysParticleCache::insert(PhysParticle* b)
{
int x = round(b->center.p[0] * oneOverCubeWidth + halfXSize);
int y = round(b->center.p[1] * oneOverCubeWidth + halfYSize);
int z = round(b->center.p[2] * oneOverCubeWidth + halfZSize);

// Get the entry from the array
ArrayItem* arrayItem = &ARRAY(x, y, z);

if(arrayItem->particlesHere == NULL)
{
clear_list[clear_list_size++] = arrayItem;
}

// Get or allocate new node
PhysParticleListNode* newNode = &particles[particles_size++];

newNode->particle = b;
newNode->next = arrayItem->particlesHere;
arrayItem->particlesHere = newNode;

insertCount ++;
}
*/

/*
// TODO: I don't really have to cache these results... I don't
// think it helps performance much, and it greatly increases the
// memory I need to store the lists of nearby particles.

// TODO: I should use a larger radius and similar masking scheme
// as for the renerer here.
PhysParticle** PhysParticleCache::get(PhysParticle* particle)
{
int x = round(particle->center.p[0] * oneOverCubeWidth + halfXSize);
int y = round(particle->center.p[1] * oneOverCubeWidth + halfYSize);
int z = round(particle->center.p[2] * oneOverCubeWidth + halfZSize);

ArrayItem* centerItem = &ARRAY(x, y, z);

if(centerItem->particlesCloseby == NULL)
{
clear_closeby_list[clear_closeby_size++] = centerItem;

centerItem->particlesCloseby = &(particles_closeby[particles_closeby_size]);

for(int i = 0; i < check_list2_size; i++)
{
// Get the entry from the array
PhysParticleListNode* node = (centerItem + check_list2[i])->particlesHere;

while(node != NULL)
{
PhysParticle* b = node->particle;

particles_closeby[particles_closeby_size++] = b;

node = node->next;
}
}

particles_closeby[particles_closeby_size++] = NULL;
}

return centerItem->particlesCloseby;
}
*/

/*
PhysParticleAndDist* PhysParticleCache::get(PhysParticle *b)
{
	return &(particles_closeby_sa[b->neighbor_list]);
}
*/