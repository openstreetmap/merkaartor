#ifndef _WIN32
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <assert.h>
#include <float.h>
using namespace std;

#include "libgosm.h"

routeNodeType *route = NULL, *shortest = NULL;
routeHeapType *routeHeap;
long dhashSize, dLength;
int routeHeapSize, tlat, tlon, flat, flon, rlat, rlon, routeHeapMaxSize;
int *hashTable, bucketsMin1, pakHead = 0xEB3A943, routeSuccess = FALSE;
char *gosmData, *gosmSstr[searchCnt];

ndType *ndBase;
styleStruct *style;
int stylecount;
wayType *gosmSway[searchCnt];

// store the maximum speeds (over all waytypes) of each vehicle type
int TagCmp (const char *a, const char *b)
{ // This works like the ordering of books in a library : We ignore
  // meaningless words like "the", "street" and "north". We (should) also map
  // deprecated words to their new words, like petrol to fuel
  // TODO : We should consider an algorithm like double metasound.
  static const char *omit[] = { /* "the", in the middle of a name ?? */
    "ave", "avenue", "blvd", "boulevard", "byp", "bypass",
    "cir", "circle", "close", "cres", "crescent", "ct", "court", "ctr",
      "center",
    "dr", "drive", "hwy", "highway", "ln", "lane", "loop",
    "pass", "pky", "parkway", "pl", "place", "plz", "plaza",
    /* "run" */ "rd", "road", "sq", "square", "st", "street",
    "ter", "terrace", "tpke", "turnpike", /*trce, trace, trl, trail */
    "walk",  "way"
  };
  static const char *words[] = { "", "first", "second", "third", "fourth",
    "fifth", "sixth", "seventh", "eighth", "nineth", "tenth", "eleventh",
    "twelth", "thirteenth", "fourteenth", "fifthteenth", "sixteenth",
    "seventeenth", "eighteenth", "nineteenth", "twentieth" };
  static const char *teens[] = { "", "", "twenty ", "thirty ", "fourty ",
    "fifty ", "sixty ", "seventy ", "eighty ", "ninety " };
  
  if (strncasecmp (a, "the ", 4) == 0) a += 4;
  else if (strncasecmp (a, "rue ", 4) == 0) a += 4;
  else if (strncasecmp (a, "avenue ", 7) == 0) a += 7;
  else if (strncasecmp (a, "boulevard ", 10) == 0) a += 10;
  if (strncasecmp (a, "de ", 3) == 0) a += 3;
  
  if (strncasecmp (b, "the ", 4) == 0) b += 4;
  else if (strncasecmp (b, "rue ", 4) == 0) b += 4;
  else if (strncasecmp (b, "avenue ", 7) == 0) b += 7;
  else if (strncasecmp (b, "boulevard ", 10) == 0) b += 10;
  if (strncasecmp (b, "de ", 3) == 0) b += 3;
  
  if (strchr ("WEST", a[0]) && a[1] == ' ') a += 2; // e.g. N 21st St
  if (strchr ("WEST", b[0]) && b[1] == ' ') b += 2;

  for (;;) {
    char n[2][30] = { "", "" };
    const char *ptr[2];
    int wl[2];
    for (int i = 0; i < 2; i++) {
      const char **p = i ? &b : &a;
      if ((*p)[0] == ' ') {
        for (int i = 0; i < int (sizeof (omit) / sizeof (omit[0])); i++) {
          if (strncasecmp (*p + 1, omit[i], strlen (omit[i])) == 0 &&
              !isalpha ((*p)[1 + strlen (omit[i])])) {
            (*p) += 1 + strlen (omit[i]);
            break;
          }
        }
      }
      if (isdigit (**p) && (!isdigit((*p)[1]) || !isdigit ((*p)[2]))
              /* && isalpha (*p + strcspn (*p, "0123456789"))*/) {
        // while (atoi (*p) > 99) (*p)++; // Buggy
        if (atoi (*p) > 20) strcpy (n[i], teens[atoi ((*p)++) / 10]);
        strcat (n[i], words[atoi (*p)]);
        while (isdigit (**p) /*|| isalpha (**p)*/) (*p)++;
        ptr[i] = n[i];
        wl[i] = strlen (n[i]);
      }
      else {
        ptr[i] = *p;
        wl[i] = **p == ' ' ? 1 : strcspn (*p , " \n");
      }
    }
    int result = strncasecmp (ptr[0], ptr[1], wl[0] < wl[1] ? wl[1] : wl[0]);
    if (result || *ptr[0] == '\0' || *ptr[0] == '\n') return result;
    if (n[0][0] == '\0') a += wl[1]; // In case b was 21st
    if (n[1][0] == '\0') b += wl[0]; // In case a was 32nd
  }
}

/* 1. Bsearch idx such that
      ZEnc (way[idx]) < ZEnc (clon/lat) < ZEnc (way[idx+1])
   2. Fill the list with ways around idx.
   3. Now there's a circle with clon/clat as its centre and that runs through
      the worst way just found. Let's say it's diameter is d. There exist
      4 Z squares smaller that 2d by 2d that cover this circle. Find them
      with binary search and search through them for the nearest ways.
   The worst case is when the nearest nodes are far along a relatively
   straight line.
*/
int *GosmIdxSearch (const char *key, unsigned z)
{
  int *idx =
    (int *)(ndBase + hashTable[bucketsMin1 + (bucketsMin1 >> 7) + 2]);
  for (int l = 0, h = hashTable - idx; ;) {
    char *tag = gosmData + idx[(h + l) / 2];
    int diff = TagCmp (tag, key);
    while (*--tag) {}
    if (diff > 0 || (diff == 0 &&
      ZEnc ((unsigned)((wayType *)tag)[-1].clat >> 16, 
            (unsigned)((wayType *)tag)[-1].clon >> 16) >= z)) h = (h + l) / 2;
    else l = (h + l) / 2 + 1;
    if (l >= h) return idx + h;
  }
}

void GosmSearch (int clon, int clat, const char *key)
{
  __int64 dista[searchCnt];
  int *idx =
    (int *)(ndBase + hashTable[bucketsMin1 + (bucketsMin1 >> 7) + 2]);
  int l = GosmIdxSearch (key, 0) - idx, count;
//  char *lastName = data + idx[min (hashTable - idx), 
//    int (sizeof (gosmSway) / sizeof (gosmSway[0]))) + l - 1];
  int cz = ZEnc ((unsigned) clat >> 16, (unsigned) clon >> 16);
  for (count = 0; count + l < hashTable - idx && count < searchCnt;) {
    int m[2], c = count, ipos, dir, bits;
    m[0] = GosmIdxSearch (gosmData + idx[count + l], cz) - idx;
    m[1] = m[0] - 1;
    __int64 distm[2] = { -1, -1 }, big = ((unsigned __int64) 1 << 63) - 1;
    while (c < searchCnt && (distm[0] < big || distm[1] < big)) {
      dir = distm[0] < distm[1] ? 0 : 1;
      if (distm[dir] != -1) {
        for (ipos = c; count < ipos && distm[dir] < dista[ipos - 1]; ipos--) {
          dista[ipos] = dista[ipos - 1];
          gosmSway[ipos] = gosmSway[ipos - 1];
          gosmSstr[ipos] = gosmSstr[ipos - 1];
        }
        char *tag = gosmData + idx[m[dir]];
        gosmSstr[ipos] = tag;
        while (*--tag) {}
        gosmSway[ipos] = (wayType*)tag - 1;
        dista[ipos] = distm[dir];
        c++;
      }
      m[dir] += dir ? 1 : -1;

      if (0 <= m[dir] && m[dir] < hashTable - idx &&
        TagCmp (gosmData + idx[m[dir]], gosmData + idx[count + l]) == 0) {
        char *tag = gosmData + idx[m[dir]];
        while (*--tag) {}
        distm[dir] = Sqr ((__int64)(clon - ((wayType*)tag)[-1].clon)) +
          Sqr ((__int64)(clat - ((wayType*)tag)[-1].clat));
      }
      else distm[dir] = big;
    }
    if (count == c) break; // Something's wrong. idx[count + l] not found !
    if (c >= searchCnt) {
      c = count; // Redo the adding
      for (bits = 0; bits < 16 && dista[searchCnt - 1] >> (bits * 2 + 32);
        bits++) {}
/* Print Z's for first solution 
      for (int j = c; j < searchCnt; j++) {
        for (int i = 0; i < 32; i++) printf ("%d%s",
          (ZEnc ((unsigned) gosmSway[j]->clat >> 16,
                 (unsigned) gosmSway[j]->clon >> 16) >> (31 - i)) & 1,
          i == 31 ? " y\n" : i % 2 ? " " : "");
      } */
/* Print centre, up, down, right and left to see if they're in the square
      for (int i = 0; i < 32; i++) printf ("%d%s", (cz >> (31 - i)) & 1,
        i == 31 ? " x\n" : i % 2 ? " " : "");
      for (int i = 0; i < 32; i++) printf ("%d%s", (
        ZEnc ((unsigned)(clat + (int) sqrt (dista[searchCnt - 1])) >> 16,
              (unsigned)clon >> 16) >> (31 - i)) & 1,
        i == 31 ? " x\n" : i % 2 ? " " : "");
      for (int i = 0; i < 32; i++) printf ("%d%s", (
        ZEnc ((unsigned)(clat - (int) sqrt (dista[searchCnt - 1])) >> 16,
              (unsigned)clon >> 16) >> (31 - i)) & 1,
        i == 31 ? " x\n" : i % 2 ? " " : "");
      for (int i = 0; i < 32; i++) printf ("%d%s", (
        ZEnc ((unsigned)clat >> 16,
              (unsigned)(clon + (int) sqrt (dista[searchCnt - 1])) >> 16) >> (31 - i)) & 1,
        i == 31 ? " x\n" : i % 2 ? " " : "");
      for (int i = 0; i < 32; i++) printf ("%d%s", (
        ZEnc ((unsigned)clat >> 16,
              (unsigned)(clon - (int) sqrt (dista[searchCnt - 1])) >> 16) >> (31 - i)) & 1,
        i == 31 ? " x\n" : i % 2 ? " " : "");
*/      
      int swap = cz ^ ZEnc (
        (unsigned) (clat + (clat & (1 << (bits + 16))) * 4 -
                                              (2 << (bits + 16))) >> 16,
        (unsigned) (clon + (clon & (1 << (bits + 16))) * 4 -
                                              (2 << (bits + 16))) >> 16);
      // Now we search through the 4 squares around (clat, clon)
      for (int mask = 0, maskI = 0; maskI < 4; mask += 0x55555555, maskI++) {
        int s = GosmIdxSearch (gosmData + idx[count + l],
          (cz ^ (mask & swap)) & ~((4 << (bits << 1)) - 1)) - idx;
/* Print the square
        for (int i = 0; i < 32; i++) printf ("%d%s", 
          (((cz ^ (mask & swap)) & ~((4 << (bits << 1)) - 1)) >> (31 - i)) & 1,
          i == 31 ? "\n" : i % 2 ? " " : "");
        for (int i = 0; i < 32; i++) printf ("%d%s", 
          (((cz ^ (mask & swap)) | ((4 << (bits << 1)) - 1)) >> (31 - i)) & 1,
          i == 31 ? "\n" : i % 2 ? " " : "");
*/
        for (;;) {
          char *tag = gosmData + idx[s++];
          if (TagCmp (gosmData + idx[count + l], tag) != 0) break;
          while (*--tag) {}
          wayType *w = (wayType*)tag - 1;
          if ((ZEnc ((unsigned)w->clat >> 16, (unsigned) w->clon >> 16) ^
               cz ^ (mask & swap)) >> (2 + (bits << 1))) break;
          __int64 d = Sqr ((__int64)(w->clat - clat)) +
                      Sqr ((__int64)(w->clon - clon));
          if (count < searchCnt || d < dista[count - 1]) {
            if (count < searchCnt) count++;
            for (ipos = count - 1; ipos > c && d < dista[ipos - 1]; ipos--) {
              dista[ipos] = dista[ipos - 1];
              gosmSway[ipos] = gosmSway[ipos - 1];
              gosmSstr[ipos] = gosmSstr[ipos - 1];
            }
            gosmSway[ipos] = w;
            dista[ipos] = d;
            gosmSstr[ipos] = gosmData + idx[s - 1];
          }
        } // For each entry in the square
      } // For each of the 4 squares
      break; // count < searchCnt implies a bug. Don't loop infinitely.
    } // If the search list is filled by tags with this text
    count = c;
  } // For each
  for (int k = count; k < searchCnt; k++) gosmSstr[k] = NULL;
}

/*------------------------------- OsmItr --------------------------------*/
int Next (OsmItr &itr) /* Friend of osmItr */
{
  do {
    itr.nd[0]++;
    while (itr.nd[0] >= itr.end) {
      if ((itr.slon += itr.tsize) == itr.right) {
        itr.slon = itr.left;  /* Here we wrap around from N85 to S85 ! */
        if ((itr.slat += itr.tsize) == itr.bottom) return FALSE;
      }
      int bucket = Hash (itr.slon, itr.slat, itr.tsize != TILESIZE);
      itr.nd[0] = ndBase + hashTable[bucket];
      itr.end = ndBase + hashTable[bucket + 1];
    }
  } while (((itr.nd[0]->lon ^ itr.slon) & (~(itr.tsize - 1))) ||
           ((itr.nd[0]->lat ^ itr.slat) & (~(itr.tsize - 1))));
/*      ((itr.hs[1] = (halfSegType *) (data + itr.hs[0]->other)) > itr.hs[0] &&
       itr.left <= itr.hs[1]->lon && itr.hs[1]->lon < itr.right &&
       itr.top <= itr.hs[1]->lat && itr.hs[1]->lat < itr.bottom)); */
/* while nd[0] is a hash collision, */ 
  return TRUE;
}

/* Routing starts at the 'to' point and moves to the 'from' point. This will
   help when we do in car navigation because the 'from' point will change
   often while the 'to' point stays fixed, so we can keep the array of nodes.
   It also makes the generation of the directions easier.

   We use "double hashing" to keep track of the shortest distance to each
   node. So we guess an upper limit for the number of nodes that will be
   considered and then multiply by a few so that there won't be too many
   clashes. For short distances we allow for dense urban road networks,
   but beyond a certain point there is bound to be farmland or seas.

   We call nodes that recently had their "best" increased "active". The
   active nodes are stored in a heap so that we can quickly find the most
   promissing one.
   
   OSM nodes are not our "graph-theor"etic nodes. Our "graph-theor"etic nodes
   are "states", namely the ability to reach nd directly from nd->other[dir]
*/
#ifdef ROUTE_CALIBRATE
int routeAddCnt;
#define ROUTE_SET_ADDND_COUNT(x) routeAddCnt = (x)
#define ROUTE_SHOW_STATS printf ("%d / %d\n", routeAddCnt, dhashSize); \
  fprintf (stderr, "flat=%lf&flon=%lf&tlat=%lf&tlon=%lf&fast=%d&v=motorcar\n", \
    LatInverse (flat), LonInverse (flon), LatInverse (tlat), \
    LonInverse (tlon), fast)
// This ratio must be around 0.5. Close to 0 or 1 is bad
#else
#define ROUTE_SET_ADDND_COUNT(x)
#define ROUTE_SHOW_STATS
#endif

static ndType *endNd[2] = { NULL, NULL}, from;
static int toEndNd[2][2];

void GosmFreeRoute (void)
{
  if (route) {
  #ifndef _WIN32
    munmap (route, (sizeof (*route)) * dhashSize);
  #else
    free (route);
  #endif
    free (routeHeap);
    route = NULL;
  }    
  routeSuccess = FALSE;
}

routeNodeType *AddNd (ndType *nd, int dir, int cost, routeNodeType *newshort)
{ /* This function is called when we find a valid route that consists of the
     segments (hs, hs->other), (newshort->hs, newshort->hs->other),
     (newshort->shortest->hs, newshort->shortest->hs->other), .., 'to'
     with cost 'cost'.
     
     When cost is -1, this function just returns the entry for nd without
     modifying anything. */
  if (nd->lat == INT_MIN) return NULL; // Nodes missing from OSM-XML
  unsigned i = 0, hash = (intptr_t) nd / 10 + dir;
  routeNodeType *n;

  //if ((routeHeapSize & 0xff) == 0) printf ("%d %d\n", dhashSize, routeHeapSize);
  do {
    if (i++ > 10) {
      //fprintf (stderr, "Double hash bailout : Table full. %9d %p\n",
      //  routeHeapSize, nd);
      // If you get the
      return NULL;
    }
    n = route + (/*(hash >> 16) ^ */hash) % dhashSize;

    hash -= hash << 6;
    hash ^= hash >> 17;
    hash -= hash << 9;
    hash ^= hash << 4;
    hash -= hash << 3;
    hash ^= hash << 10;
    hash ^= hash >> 15;

//    hash = (unsigned) (hash * (__int64) 1664525 + 1013904223);
            
    if (n->nd == NULL) { /* First visit of this node */
      if (cost < 0) return NULL;
      if (routeHeapSize >= routeHeapMaxSize) {
        //printf ("Route Heap too big\n");
        return NULL;
      }
      n->nd = nd;
      routeHeap[routeHeapSize].best = 0x7fffffff;
      /* Will do later : routeHeap[routeHeapSize].r = n; */
      n->heapIdx = routeHeapSize++;
      n->dir = dir;
      n->remain = lrint (sqrt (Sqr ((__int64)(nd->lat - rlat)) +
                               Sqr ((__int64)(nd->lon - rlon))));
      if (!shortest || n->remain < shortest->remain) shortest = n;

      ROUTE_SET_ADDND_COUNT (routeAddCnt + 1);
    }
  } while (n->nd != nd || n->dir != dir);

  routeHeapType h;
  h.r = n;
  h.best = cost + n->remain - (!newshort ? 0 : newshort->heapIdx < 0
    ? newshort->remain + newshort->heapIdx
    : newshort->remain - routeHeap[newshort->heapIdx].best);
  // TODO make sure newshort is never in the heap and exploit it.
  if (cost >= 0 && (n->heapIdx < 0 ? -n->heapIdx : routeHeap[n->heapIdx].best)
                   > h.best) {
    n->shortest = newshort;
    if (n->heapIdx < 0) n->heapIdx = routeHeapSize++;
    for (; n->heapIdx > 1 && h.best < routeHeap[n->heapIdx / 2].best; n->heapIdx /= 2) {
      memcpy (routeHeap + n->heapIdx, routeHeap + n->heapIdx / 2,
        sizeof (*routeHeap));
      routeHeap[n->heapIdx].r->heapIdx = n->heapIdx;
    }
    memcpy (&routeHeap[n->heapIdx], &h, sizeof (routeHeap[n->heapIdx]));
  }
  return n;
}

inline int IsOneway (wayType *w, int Vehicle)
{
  return Vehicle != footR &&
    (w->bits & (1 << (Vehicle == bicycleR ? bicycleOneway : onewayR)));
  //!((Vehicle == footR || Vehicle == bicycleR) &&
  //  (w->bits & (1 << motorcarR))) && (w->bits & (1<<onewayR));
}

// left-hand drive country aproximate bounding boxes
static const int lhtBbox[][4] = {
  { Longitude (10.2), Latitude (-85.0), Longitude (42.1), Latitude (4.7) },
  // Africa. Not correct for Angola, DRC and a few other poor countries.
  { Longitude (-14.11), Latitude (49.83), Longitude (1.84), Latitude (60.03) },
  // UK & IE
  { Longitude (68.0), Latitude (0.0), Longitude (90.2), Latitude (31.4) },
  // India & Sri Lanka
  { Longitude (9.3), Latitude (-85.0), Longitude (179.0), Latitude (19.1) },
  // Aus, NZ, Indonesia, Malazia, Thailand.
  { Longitude (129.55), Latitude (18.0), Longitude (145.84), Latitude (45.55) }
  // Japan
};

static int lht = FALSE, Vehicle, fast;

void Route (int recalculate, int plon, int plat, int _vehicle, int _fast)
{ /* Recalculate is faster but only valid if 'to', 'Vehicle' and
     'fast' did not change */
/* We start by finding the segment that is closest to 'from' and 'to' */
  ROUTE_SET_ADDND_COUNT (0);
  shortest = NULL;
  routeSuccess = FALSE;
  Vehicle = _vehicle;
  fast = _fast;
  for (int i = recalculate ? 0 : 1; i < 2; i++) {
    int lon = i ? flon : tlon, lat = i ? flat : tlat;
    __int64 bestd = (__int64) 1 << 62;
    /* find min (Sqr (distance)). Use long long so we don't loose accuracy */
    OsmItr itr (lon - 100000, lat - 100000, lon + 100000, lat + 100000);
    /* Search 1km x 1km around 'from' for the nearest segment to it */
    while (Next (itr)) {
      // We don't do for (int dir = 0; dir < 1; dir++) {
      // because if our search box is large enough, it will also give us
      // the other node.
      if (!(Way (itr.nd[0])->bits & (1 << Vehicle))) {
        continue;
      }
      if (itr.nd[0]->other[0] == 0) continue;
      __int64 lon0 = lon - itr.nd[0]->lon, lat0 = lat - itr.nd[0]->lat,
              lon1 = lon - (itr.nd[0] + itr.nd[0]->other[0])->lon,
              lat1 = lat - (itr.nd[0] + itr.nd[0]->other[0])->lat,
              dlon = lon1 - lon0, dlat = lat1 - lat0;
      /* We use Pythagoras to test angles for being greater that 90 and
         consequently if the point is behind hs[0] or hs[1].
         If the point is "behind" hs[0], measure distance to hs[0] with
         Pythagoras. If it's "behind" hs[1], use Pythagoras to hs[1]. If
         neither, use perpendicular distance from a point to a line */
      int segLen = lrint (sqrt ((double)(Sqr(dlon) + Sqr (dlat))));
      __int64 d = dlon * lon0 >= - dlat * lat0 ? Sqr (lon0) + Sqr (lat0) :
        dlon * lon1 <= - dlat * lat1 ? Sqr (lon1) + Sqr (lat1) :
        Sqr ((dlon * lat1 - dlat * lon1) / segLen);
      
      wayType *w = Way (itr.nd[0]);
      if (i) { // For 'from' we take motion into account
        __int64 motion = segLen ? 3 * (dlon * plon + dlat * plat) / segLen
          : 0;
        // What is the most appropriate multiplier for motion ?
        if (motion > 0 && IsOneway (w, Vehicle)) d += Sqr (motion);
        else d -= Sqr (motion);
        // Is it better to say :
        // d = lrint (sqrt ((double) d));
        // if (motion < 0 || IsOneway (w)) d += motion;
        // else d -= motion; 
      }
      
      if (d < bestd) {
        bestd = d;
        double invSpeed = !fast ? 1.0 : Style (w)->invSpeed[Vehicle];
        //printf ("%d %lf\n", i, invSpeed);
        toEndNd[i][0] =
          lrint (sqrt ((double)(Sqr (lon0) + Sqr (lat0))) * invSpeed);
        toEndNd[i][1] =
          lrint (sqrt ((double)(Sqr (lon1) + Sqr (lat1))) * invSpeed);
//        if (dlon * lon1 <= -dlat * lat1) toEndNd[i][1] += toEndNd[i][0] * 9;
//        if (dlon * lon0 >= -dlat * lat0) toEndNd[i][0] += toEndNd[i][1] * 9;

        if (IsOneway (w, Vehicle)) toEndNd[i][1 - i] = 200000000;
        /*  It's possible to go up a oneway at the end, but at a huge penalty*/
        endNd[i] = itr.nd[0];
        /* The router only stops after it has traversed endHs[1], so if we
           want 'limit' to be accurate, we must subtract it's length
        if (i) {
          toEndHs[1][0] -= segLen; 
          toEndHs[1][1] -= segLen;
        } */
      }
    } /* For each candidate segment */
    if (bestd == ((__int64) 1 << 62) || !endNd[0]) {
      endNd[i] = NULL;
      //fprintf (stderr, "No segment nearby\n");
      return;
    }
  } /* For 'from' and 'to', find segment that passes nearby */
  from.lat = flat;
  from.lon = flon;
  if (recalculate || !route) {
    GosmFreeRoute ();
    routeHeapSize = 1; /* Leave position 0 open to simplify the math */
    #ifndef _WIN32
    // Google "zero-fill-on-demand". Basically all the reasons they mention are valid.
    // In particular, while gosmore is paused while a page is swapped in, the OS can
    // zero some pages for us.
    int dzero = open ("/dev/zero", O_RDWR);
    long long ds = sysconf (_SC_PAGESIZE) * (long long) sysconf (_SC_PHYS_PAGES) /
      (sizeof (*routeHeap) + sizeof (*route) + 40);
    dhashSize = ds > INT_MAX ? INT_MAX : ds;
    routeHeapMaxSize = lrint (sqrt (dhashSize)) * 3;
    routeHeap = (routeHeapType*) malloc (routeHeapMaxSize * sizeof (*routeHeap));
    if (!routeHeap || dzero == -1 ||
        (route = (routeNodeType*) mmap (NULL, dhashSize * sizeof (*route),
        PROT_READ | PROT_WRITE, MAP_SHARED, dzero, 0)) == MAP_FAILED) {
      fprintf (stderr, "Error: Mmap of dnull for routing arrays\n");
      route = NULL;
      return;
    }
    if (dzero != -1) close (dzero);
    #else
    MEMORYSTATUS memStat;
    GlobalMemoryStatus (&memStat);
    dhashSize = (memStat.dwAvailPhys - 400000) /
                 (sizeof (*route) + sizeof (*routeHeap) + 40);

    routeHeapMaxSize = lrint (sqrt (dhashSize)) * 3;
    routeHeap = (routeHeapType*) malloc (routeHeapMaxSize * sizeof (*routeHeap));
    if (!routeHeap) return;
    if (!(route = (routeNodeType*) calloc (sizeof (*route), dhashSize))) return;
    #endif

    rlat = flat;
    rlon = flon;
    AddNd (endNd[0], 0, toEndNd[0][0], NULL);
    AddNd (endNd[0] + endNd[0]->other[0], 1, toEndNd[0][1], NULL);
    AddNd (endNd[0], 1, toEndNd[0][0], NULL);
    AddNd (endNd[0] + endNd[0]->other[0], 0, toEndNd[0][1], NULL);

    #ifdef INSPECT
    printf ("\ncycleonewa: %s\n",
      Way (endNd[0])->bits & (1 << bicycleOneway) ? "yes" : "no");
    #define M(x) printf ("%10s: %s\n", #x, \
                   Way (endNd[0])->bits & (1 << x ## R) ? "yes" : "no");
    RESTRICTIONS
    #undef M
    // A bit confusing when the user clicks a way on which the selected
    // vehicle type is not allowed, because endNd will then be a different
    // way.
    #endif
  }
  else {
    routeNodeType *frn = AddNd (&from, 0, -1, NULL);
    if (frn) {
      if (frn->heapIdx < 0) frn->heapIdx = -0x7fffffff;
      else routeHeap[frn->heapIdx].best = 0x7fffffff;
    }

    routeNodeType *rn = AddNd (endNd[1], 0, -1, NULL);
    if (rn) AddNd (&from, 0, toEndNd[1][1], rn);
    routeNodeType *rno = AddNd (endNd[1] + endNd[1]->other[0], 1, -1, NULL);
    if (rno) AddNd (&from, 0, toEndNd[1][0], rno);
  }
  lht = FALSE;
  // keep lht true if we are not in any of the lhtBbox
  for (size_t i = 0; i < sizeof (lhtBbox) / sizeof (lhtBbox[0]); i++) {
    lht = lht || (lhtBbox[i][0] < tlon && lhtBbox[i][1] < tlat &&
                  tlon < lhtBbox[i][2] && tlat < lhtBbox[i][3]);
  }
  // printf (lht ? "Left Hand Traffic\n" : "Right Hand Traffic\n");
}

int RouteLoop (void)
{
  // printf ("%ld %ld\n", clock (), CLOCKS_PER_SEC); // Calibrate loop below
  for (int i = 0; i < 20000 && routeHeapSize > 1; i++) {
    routeNodeType *root = routeHeap[1].r;
    root->heapIdx = -routeHeap[1].best; /* Root now removed from the heap */
    routeHeapSize--;
    for (int i = 2; i / 2 < routeHeapSize; ) {
      routeHeapType *end = &routeHeap[routeHeapSize];
      int sml = i >= routeHeapSize || routeHeap[i].best > end->best
        ? (i + 1 >= routeHeapSize || routeHeap[i].best > end->best
           ? routeHeapSize : i + 1)
        : routeHeap[i].best < routeHeap[i + 1].best ? i : i + 1;
      memcpy (routeHeap + i / 2, routeHeap + sml, sizeof (*routeHeap));
      routeHeap[i / 2].r->heapIdx = i / 2;
      i = sml * 2;
    }
    if (root->nd == &from) { // Remove 'from' from the heap in case we
      shortest = root->shortest; // get called with recalculate=0
      routeSuccess = TRUE;
      return FALSE;
    }
    if (root->nd == (!root->dir ? endNd[1] : endNd[1] + endNd[1]->other[0])) {
      AddNd (&from, 0, toEndNd[1][1 - root->dir], root);
    }
    ndType *nd = root->nd, *other, *firstNd, *restrictItr;
    while (nd > ndBase && nd[-1].lon == nd->lon &&
      nd[-1].lat == nd->lat) nd--; /* Find first nd in node */
    firstNd = nd; // Save it for checking layout and restrictions
    int rootIsAdestination = Way (root->nd)->destination & (1 << Vehicle);
    /* Now work through the segments connected to root. */
    
    unsigned layout[4] = { 0, 0, 0, 0 }, lmask = 1;
    ndType *rtother = root->nd + root->nd->other[root->dir], *layoutNd[4];
    // We categorize each segment leaving this junction according to the
    // angle it form with the 'root' segment. The angles are 315 to 45,
    // 45 to 135, 135 to 225 and 225 to 315 degrees.
    do {
      lmask <<= 2;
      for (int dir = 0; dir < 2; dir++) {
        if (nd->other[dir] != 0) {
          other = nd + nd->other[dir];
          int dot = (other->lat - nd->lat) * (nd->lat - rtother->lat) +
                    (other->lon - nd->lon) * (nd->lon - rtother->lon);
          int cross = (other->lat - nd->lat) * (nd->lon - rtother->lon) -
                      (other->lon - nd->lon) * (nd->lat - rtother->lat);
          int azimuth = (dot > cross ? 0 : 3) ^ (dot + cross > 0 ? 0 : 1);
          layout[azimuth] |= lmask << dir;
          layoutNd[azimuth] = nd;
        }
      }
    } while (++nd < ndBase + hashTable[bucketsMin1 + 1] &&
             nd->lon == nd[-1].lon && nd->lat == nd[-1].lat);
             
    //printf ("%d %d %d %d\n", layout[0], layout[1], layout[2], layout[3]);
    nd = firstNd;
    lmask = 1;
    do {
      if (StyleNr (Way (nd)) >= barrier_bollard &&
          StyleNr (Way (nd)) <= barrier_toll_booth &&
          !(Way (nd)->bits & (1 << Vehicle))) break;
      lmask <<= 2;
      #ifdef _WIN32_WCE
      if (root->remain > 5500000 && -root->heapIdx - root->remain > 5500000 &&
      #else
      if (root->remain > 25500000 && -root->heapIdx - root->remain > 25500000 &&
      #endif
          (StyleNr (Way (nd)) == highway_residential ||
           StyleNr (Way (nd)) == highway_service ||
           StyleNr (Way (nd)) == highway_living_street ||
           StyleNr (Way (nd)) == highway_unclassified)) continue;
      /* When more than 75km from the start and the finish, ignore minor
         roads. This reduces the number of calculations. */
      for (int dir = 0; dir < 2; dir++) {
        if (nd == root->nd && dir == root->dir) continue;
        /* Don't consider an immediate U-turn to reach root->hs->other.
           This doesn't exclude 179.99 degree turns though. */
        
        if (nd->other[dir] == 0) continue;
        if (Vehicle != footR && Vehicle != bicycleR) {
          for (restrictItr = firstNd; restrictItr->other[0] == 0 &&
                          restrictItr->other[1] == 0; restrictItr++) {
            wayType *w  = Way (restrictItr);
            if (StyleNr (w) >= restriction_no_right_turn &&
                StyleNr (w) <= restriction_only_straight_on &&
                atoi ((char*)(w + 1) + 1) == nd->wayPtr &&
            (StyleNr (w) <= restriction_no_straight_on) ==
            (atoi (strchr ((char*)(w + 1) + 1, ' ')) == root->nd->wayPtr)) {
              break;
            }
          }
          if (restrictItr->other[0] == 0 &&
              restrictItr->other[1] == 0) continue;
        }
        // Tagged node, start or end of way or restriction.
        
        other = nd + nd->other[dir];
        wayType *w = Way (nd);

	// // DEBUGGING
	// char label[255], *strings;
	// strings=(char*)(w+1)+1;
	// memset(label,0,255);
	// strncpy(label,strings,strcspn(strings,"\n"));
	// printf ("DEBUG: %s (%d) %lf\n", label, StyleNr(w), 
	// 	Style(w)->invSpeed[Vehicle]);


        int myV = Vehicle == bicycleR && (!(w->bits & (1 << bicycleR)) 
          || ((w->bits & (1 << bicycleOneway)) && !dir)) ? footR : Vehicle;
        // If pedestrians are allowed and cyclists not, we can dismount and
        // walk. The same applies when we can't cycle in the wrong direction.
        if ((w->bits & (1 << myV)) && (dir || !IsOneway (w, myV))) {
          int d = lrint (sqrt ((double)
            (Sqr ((__int64)(nd->lon - other->lon)) +
             Sqr ((__int64)(nd->lat - other->lat)))) *
            (!fast ? 1.0 : Style (w)->invSpeed[Vehicle]));
          if (Vehicle != myV) d *= 4; // Penalty for dismounting
          if (rootIsAdestination && !(w->destination & (1 << Vehicle))) {
            d += 5000000; // 500km penalty for entering v='destination' area.
          }
          
          // If (lmask<<dir)&layout[x] is set, we are going approximately
          // in direction x * 90 degrees, relative to the direction we
          // are going to (Remember that we are coming from 'root')
          // If layout[x] is not zero, there are segments going in that
          // direction
          if (layout[lht ? 1 : 3] && ((lmask << dir) & layout[lht ? 3 : 1])
              && fast && Style (w)->scaleMax > 100000) {
            d += 50000 * (fast ? Style (w)->invSpeed[Vehicle] : 1);
          // Turning right in the UK (or left in the rest of the world), when
          // we are on a major road (secondary+) that continues straight on,
          // you will probably have to wait for oncoming traffic.
          }
          
          if (layout[1] && layout[3] && ((lmask << dir) & layout[0])) {
            // Straight over a T-junction
            if ((Way (layoutNd[1])->bits & (1 << motorcarR)) &&
                (Way (layoutNd[3])->bits & (1 << motorcarR)) && fast) {
            // And motorcars are allowed on both sides
              d += (Style (Way (layoutNd[1]))->invSpeed[motorcarR] <
                    Style (w)->invSpeed[motorcarR] ? 10000 : 3000) *
                    (fast ? Style (w)->invSpeed[Vehicle] : 1);
            // Crossing a road that is faster that the road we are traveling
            // on incurs a 100m penalty. If they are equal, the penality is
            // 30m. TODO: residential crossing residential should be less,
            // perhaps 20m.
            }
          }
          
          ndType *n2 = root->nd + root->nd->other[root->dir];
          __int64 straight =
            (n2->lat - nd->lat) * (__int64)(nd->lat - other->lat) +
            (n2->lon - nd->lon) * (__int64)(nd->lon - other->lon), left =
            (n2->lat - nd->lat) * (__int64)(nd->lon - other->lon) -
            (n2->lon - nd->lon) * (__int64)(nd->lat - other->lat);
          // Note: We need not calculate these, because we can check the relevant
          // bits in layout
            
          // The code below adds a penalty to making a U-turn when an oncoming road
          // exists for example where a bidirectional road splits into two oneways.
          // We can add an extra condition to allow a right turn in right hand
          // traffic countries. Something like '&& lht ? left : -left > 0)'
          
          // Currently the threshold angle is 45 degrees (a turn of more than 135 degrees).
          // We can vary it using this line:
          // straight = straight * lrint (tan (M_PI / 180 * 45) * 100) / 100;
          if (straight < -left && straight < left && layout[2]) d += 50000 *
            (fast ? Style (w)->invSpeed[Vehicle] : 1);
          // Note: The layout values I've seen during debugging didn't add up.
          
          AddNd (other, 1 - dir, d, root);
        } // If we found a segment we may follow
      } // for each direction
    } while (++nd < ndBase + hashTable[bucketsMin1 + 1] &&
             nd->lon == nd[-1].lon && nd->lat == nd[-1].lat);
  } // While there are active nodes left
  ROUTE_SHOW_STATS;
//  if (fastest) printf ("%lf
//  printf ("%lf km\n", limit / 100000.0);
  return routeHeapSize > 1;
}

int JunctionType (ndType *nd)
{
  int ret = 'j';
  while (nd > ndBase && nd[-1].lon == nd->lon &&
    nd[-1].lat == nd->lat) nd--;
  int segCnt = 0; // Count number of segments at x->shortest
  do {
    // TODO : Only count segment traversable by 'Vehicle'
    // Except for the case where a cyclist passes a motorway_link.
    // TODO : Don't count oneways entering the roundabout
    if (nd->other[0] != 0) segCnt++;
    if (nd->other[1] != 0) segCnt++;
    if (StyleNr (Way (nd)) == highway_traffic_signals) {
      ret = 't';
    }
    if (StyleNr (Way (nd)) == highway_mini_roundabout) {
      ret = 'm';
    }   
  } while (++nd < ndBase + hashTable[bucketsMin1 + 1] &&
           nd->lon == nd[-1].lon && nd->lat == nd[-1].lat);
  return segCnt > 2 ? toupper (ret) : ret;
}

int GosmInit (void *d, long size)
{
  if (!d) return FALSE;
  gosmData = (char*) d;
  ndBase = (ndType *)(gosmData + ((int*)(gosmData + size))[-1]);
  bucketsMin1 = ((int *) (gosmData + size))[-2];
  stylecount = ((int *) (gosmData + size))[-3];
  style = (struct styleStruct *)
    (gosmData + size - sizeof (stylecount) * 3) - stylecount;
  hashTable = (int *) (style) - bucketsMin1 - (bucketsMin1 >> 7) - 3;
  
  memset (gosmSway, 0, sizeof (gosmSway));

  return ndBase && hashTable && *(int*) gosmData == pakHead;
}

