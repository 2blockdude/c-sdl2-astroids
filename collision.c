#include <math.h>
#include <stdlib.h>

struct point
{
   float x, y;
};

static int fpolycmp(const void *a, const void *b)
{
   return *(float *)a - *(float *)b;
}

int point_polygon_collision(float x, float y, const float *polygon, int nsides)
{
   if (polygon == NULL)
      return 0;

   struct point *points = (struct point *)polygon;

   float max_y = points[0].y;
   float min_y = points[0].y;

   for (int i = 0; i < nsides; i ++)
   {
      max_y = points[i].y > max_y ? points[i].y : max_y;
      min_y = points[i].y < min_y ? points[i].y : min_y;
   }

   int nint;
   float *nodes_x = (float *)malloc(sizeof(float) * nsides);

   nint = 0;

   //  Build a list of nodes.
   int ind1;
   int ind2;
   for (int i = 0; i < nsides; i++)
   {
      float x1;
      float y1;
      float x2;
      float y2;

      // get current point at i == 0
      if (i == 0) {
         ind1 = nsides - 1;
         ind2 = 0;
      } else {
         ind1 = i - 1;
         ind2 = i;
      }

      y1 = points[ind1].y;
      y2 = points[ind2].y;
      if (y1 < y2) {
         x1 = points[ind1].x;
         x2 = points[ind2].x;
      } else if (y1 > y2) {
         y2 = points[ind1].y;
         y1 = points[ind2].y;
         x2 = points[ind1].x;
         x1 = points[ind2].x;
      } else {
         continue;
      }
      if ( ((y >= y1) && (y < y2)) || ((y == max_y) && (y > y1) && (y <= y2)) )
      {
         nodes_x[nint++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
      }
   }

   qsort(nodes_x, nint, sizeof(float), fpolycmp);

   // check if point is inside this
   if (nint % 2 == 0)
   {
      for (int k = 0; k < nint; k += 2)
      {
         if (x > nodes_x[k] && x < nodes_x[k + 1])
            return 1;
      }
   }

   free(nodes_x);

   return 0;
}

int polygon_polygon_collision(const float *p1, int nsides1, const float *p2, int nsides2)
{
   struct point *points1 = (struct point *)p1;
   struct point *points2 = (struct point *)p2;

   // see if any point of p1 is inside p2
   for (int i = 0; i < nsides1; i++)
   {
      if (point_polygon_collision(points1[i].x, points1[i].y, p2, nsides2))
         return 1;
   }

   // see if any point of p2 is inside p1
   for (int i = 0; i < nsides2; i++)
   {
      if (point_polygon_collision(points2[i].x, points2[i].y, p1, nsides1))
         return 1;
   }

   return 0;
}
