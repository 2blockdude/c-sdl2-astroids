#ifndef COLLISION_DETECTION
#define COLLISION_DETECTION

int point_polygon_collision      (float x, float y, const float *polygon, int nsides);
int polygon_polygon_collision    (const float *p1, int nsides1, const float *p2, int nsides2);

#endif
