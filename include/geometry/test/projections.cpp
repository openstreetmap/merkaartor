// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>


#include "common.hpp"

#include <geometry/geometries/adapted/c_array.hpp>
#include <geometry/algorithms/transform.hpp>

#include <geometry/projections/factory.hpp>
#include <geometry/projections/parameters.hpp>

#include <geometry/geometries/geometries.hpp>
#include <geometry/io/wkt/streamwkt.hpp>


template <typename P>
void test_one(double lon, double lat,
			  typename geometry::coordinate_type<P>::type x,
			  typename geometry::coordinate_type<P>::type y,
			  const std::string& parameters)
{
	typedef typename geometry::coordinate_type<P>::type T;

	typedef geometry::point_ll<T, geometry::cs::geographic<geometry::degree> > lola;

	lola ll;
	ll.lon(lon);
	ll.lat(lat);

	projection::parameters par = projection::impl::pj_init_plus(parameters);
	projection::factory<lola, P, projection::parameters> fac;

	boost::shared_ptr<projection::projection<lola, P> > prj(fac.create_new(par));

	P xy;
	prj->forward(ll, xy);

	BOOST_CHECK_CLOSE(geometry::get<0>(xy), x, 0.001);
	BOOST_CHECK_CLOSE(geometry::get<1>(xy), y, 0.001);
}



template <typename P>
void test_all()
{
	/* aea */ test_one<P>(4.897000, 52.371000, 334609.583974, 5218502.503686, "+proj=aea +ellps=WGS84 +units=m +lat_1=55 +lat_2=65");
	/* aeqd */ test_one<P>(4.897000, 52.371000, 384923.723428, 5809986.497118, "+proj=aeqd +ellps=WGS84 +units=m");
	/* airy */ test_one<P>(4.897000, 52.371000, 328249.003313, 4987937.101447, "+proj=airy +ellps=WGS84 +units=m");
	/* aitoff */ test_one<P>(4.897000, 52.371000, 384096.182830, 5831239.274680, "+proj=aitoff +ellps=WGS84 +units=m");
	/* alsk */ test_one<P>(-84.390000, 33.755000, 7002185.416415, -3700467.546545, "+proj=alsk +ellps=WGS84 +units=m");
	/* apian */ test_one<P>(4.897000, 52.371000, 360906.947408, 5829913.052335, "+proj=apian +ellps=WGS84 +units=m");
	/* august */ test_one<P>(4.897000, 52.371000, 472494.816642, 6557137.075680, "+proj=august +ellps=WGS84 +units=m");
	/* bacon */ test_one<P>(4.897000, 52.371000, 203584.432463, 7934660.138798, "+proj=bacon +ellps=WGS84 +units=m");
	/* bipc */ test_one<P>(4.897000, 52.371000, 3693973.674143, -8459972.647559, "+proj=bipc +ellps=WGS84 +units=m");
	/* boggs */ test_one<P>(4.897000, 52.371000, 346620.618258, 5966441.169806, "+proj=boggs +ellps=WGS84 +units=m");
	/* bonne */ test_one<P>(4.897000, 52.371000, 333291.091896, 274683.016972, "+proj=bonne +ellps=WGS84 +units=m +lat_1=50");
	/* cass */ test_one<P>(4.897000, 52.371000, 333274.431072, 5815921.803069, "+proj=cass +ellps=WGS84 +units=m");
	/* cc */ test_one<P>(4.897000, 52.371000, 545131.546415, 8273513.720038, "+proj=cc +ellps=WGS84 +units=m");
	/* cea */ test_one<P>(4.897000, 52.371000, 545131.546415, 5031644.669407, "+proj=cea +ellps=WGS84 +units=m");
	/* chamb */ test_one<P>(4.897000, 52.371000, -3221300.532044, 872840.127676, "+proj=chamb +ellps=WGS84 +units=m +lat_1=52 +lon_1=5 +lat_2=30 +lon_2=80 +lat_3=20 +lon_3=-50");
	/* collg */ test_one<P>(4.897000, 52.371000, 280548.640940, 6148862.475491, "+proj=collg +ellps=WGS84 +units=m");
	/* crast */ test_one<P>(4.897000, 52.371000, 340944.220871, 5874029.522010, "+proj=crast +ellps=WGS84 +units=m");
	/* denoy */ test_one<P>(4.897000, 52.371000, 382253.324398, 5829913.052335, "+proj=denoy +ellps=WGS84 +units=m");
	/* eck1 */ test_one<P>(4.897000, 52.371000, 356112.818167, 5371202.270688, "+proj=eck1 +ellps=WGS84 +units=m");
	/* eck2 */ test_one<P>(4.897000, 52.371000, 320023.223588, 6697754.654662, "+proj=eck2 +ellps=WGS84 +units=m");
	/* eck3 */ test_one<P>(4.897000, 52.371000, 417367.858470, 4923223.990430, "+proj=eck3 +ellps=WGS84 +units=m");
	/* eck4 */ test_one<P>(4.897000, 52.371000, 383678.300021, 6304427.033917, "+proj=eck4 +ellps=WGS84 +units=m");
	/* eck5 */ test_one<P>(4.897000, 52.371000, 387191.346304, 5142132.228246, "+proj=eck5 +ellps=WGS84 +units=m");
	/* eck6 */ test_one<P>(4.897000, 52.371000, 342737.885307, 6363364.830847, "+proj=eck6 +ellps=WGS84 +units=m");
	/* eqc */ test_one<P>(4.897000, 52.371000, 545131.546415, 5829913.052335, "+proj=eqc +ellps=WGS84 +units=m");
	/* eqdc */ test_one<P>(4.897000, 52.371000, 307874.536263, 5810915.646438, "+proj=eqdc +ellps=WGS84 +units=m +lat_1=60 +lat_2=0");
	/* euler */ test_one<P>(4.897000, 52.371000, 338753.024859, 5836825.984893, "+proj=euler +ellps=WGS84 +units=m +lat_1=60 +lat_2=0");
	/* fahey */ test_one<P>(4.897000, 52.371000, 388824.354103, 5705638.873094, "+proj=fahey +ellps=WGS84 +units=m");
	/* fouc */ test_one<P>(4.897000, 52.371000, 268017.369817, 6272855.564674, "+proj=fouc +ellps=WGS84 +units=m");
	/* fouc_s */ test_one<P>(4.897000, 52.371000, 545131.546415, 5051361.531375, "+proj=fouc_s +ellps=WGS84 +units=m");
	/* gall */ test_one<P>(4.897000, 52.371000, 385466.213109, 5354217.135929, "+proj=gall +ellps=WGS84 +units=m");
	/* geocent */ test_one<P>(4.897000, 52.371000, 545131.546415, 5829913.052335, "+proj=geocent +ellps=WGS84 +units=m");
	/* geos */ test_one<P>(4.897000, 52.371000, 313594.638994, 4711397.361812, "+proj=geos +ellps=WGS84 +units=m +h=40000000");
	/* gins8 */ test_one<P>(4.897000, 52.371000, 409919.989781, 6235811.415629, "+proj=gins8 +ellps=WGS84 +units=m");
	/* gnom */ test_one<P>(4.897000, 52.371000, 546462.815658, 8303824.612633, "+proj=gnom +ellps=WGS84 +units=m");
	/* goode */ test_one<P>(4.897000, 52.371000, 360567.451176, 5782693.787691, "+proj=goode +ellps=WGS84 +units=m");
	/* gs48 */ test_one<P>(-84.390000, 33.755000, 4904886.323054, 12421187.782392, "+proj=gs48 +ellps=WGS84 +units=m");
	/* gs50 */ test_one<P>(-84.390000, 33.755000, 3190310.148850, -564230.076744, "+proj=gs50 +ellps=WGS84 +units=m");
	/* gstmerc */ test_one<P>(4.897000, 52.371000, 333173.875017, 5815062.181746, "+proj=gstmerc +ellps=WGS84 +units=m");
	/* hammer */ test_one<P>(4.897000, 52.371000, 370843.923425, 5630047.232233, "+proj=hammer +ellps=WGS84 +units=m");
	/* hatano */ test_one<P>(4.897000, 52.371000, 383644.128560, 6290117.704632, "+proj=hatano +ellps=WGS84 +units=m");
	/* kav5 */ test_one<P>(4.897000, 52.371000, 383646.088858, 5997047.888175, "+proj=kav5 +ellps=WGS84 +units=m");
	/* kav7 */ test_one<P>(4.897000, 52.371000, 407769.043907, 5829913.052335, "+proj=kav7 +ellps=WGS84 +units=m");
	/* krovak */ test_one<P>(4.897000, 52.371000, -1343424.322189, -661733.505873, "+proj=krovak +ellps=WGS84 +units=m");
	/* labrd */ test_one<P>(47.516667, -18.916667, 513667.401013, 797810.774147, "+proj=labrd +ellps=intl +azi=18d54 +lat_0=18d54S +lon_0=46d26'13.95E +k_0=0.9995 +x_0=400000 +y_0=800000");
	/* laea */ test_one<P>(4.897000, 52.371000, 371541.476735, 5608007.251007, "+proj=laea +ellps=WGS84 +units=m");
	/* lagrng */ test_one<P>(4.897000, 52.371000, 413379.673720, 6281547.821085, "+proj=lagrng +ellps=WGS84 +units=m +W=1");
	/* larr */ test_one<P>(4.897000, 52.371000, 485541.716273, 6497324.523196, "+proj=larr +ellps=WGS84 +units=m");
	/* lask */ test_one<P>(4.897000, 52.371000, 456660.618715, 6141427.377857, "+proj=lask +ellps=WGS84 +units=m");
	/* latlon */ test_one<P>(4.897000, 52.371000, 0.085469, 0.914046, "+proj=latlon +ellps=WGS84 +units=m");
	/* latlong */ test_one<P>(4.897000, 52.371000, 0.085469, 0.914046, "+proj=latlong +ellps=WGS84 +units=m");
	/* lcc */ test_one<P>(4.897000, 52.371000, 319700.820572, 5828852.504871, "+proj=lcc +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
	/* lcca */ test_one<P>(4.897000, 52.371000, 363514.402883, 2555324.493896, "+proj=lcca +ellps=WGS84 +units=m +lat_0=30n +lat_1=55n +lat_2=60n");
	/* leac */ test_one<P>(4.897000, 52.371000, 249343.870798, 6909632.226405, "+proj=leac +ellps=WGS84 +units=m");
	/* lee_os */ test_one<P>(4.897000, 52.371000, 2470789.632346, 31997156.368301, "+proj=lee_os +ellps=WGS84 +units=m");
	/* longlat */ test_one<P>(4.897000, 52.371000, 0.085469, 0.914046, "+proj=longlat +ellps=WGS84 +units=m");
	/* lonlat */ test_one<P>(4.897000, 52.371000, 0.085469, 0.914046, "+proj=lonlat +ellps=WGS84 +units=m");
	/* loxim */ test_one<P>(4.897000, 52.371000, 462770.371742, 5829913.052335, "+proj=loxim +ellps=WGS84 +units=m");
	/* lsat */ test_one<P>(4.897000, 52.371000, 11994414.545228, 3758147.909029, "+proj=lsat +ellps=WGS84 +units=m +lsat=1 +path=1");
	/* mbt_fps */ test_one<P>(4.897000, 52.371000, 392815.792409, 6007058.470101, "+proj=mbt_fps +ellps=WGS84 +units=m");
	/* mbt_s */ test_one<P>(4.897000, 52.371000, 389224.301381, 5893467.204064, "+proj=mbt_s +ellps=WGS84 +units=m");
	/* mbtfpp */ test_one<P>(4.897000, 52.371000, 345191.582111, 6098551.031494, "+proj=mbtfpp +ellps=WGS84 +units=m");
	/* mbtfpq */ test_one<P>(4.897000, 52.371000, 371214.469979, 5901319.366034, "+proj=mbtfpq +ellps=WGS84 +units=m");
	/* mbtfps */ test_one<P>(4.897000, 52.371000, 325952.066750, 6266156.827884, "+proj=mbtfps +ellps=WGS84 +units=m");
	/* merc */ test_one<P>(4.897000, 52.371000, 545131.546415, 6833623.829215, "+proj=merc +ellps=WGS84 +units=m");
	/* mil_os */ test_one<P>(4.897000, 52.371000, -1017212.552960, 3685935.358004, "+proj=mil_os +ellps=WGS84 +units=m");
	/* mill */ test_one<P>(4.897000, 52.371000, 545131.546415, 6431916.372717, "+proj=mill +ellps=WGS84 +units=m");
	/* moll */ test_one<P>(4.897000, 52.371000, 360567.451176, 6119459.421291, "+proj=moll +ellps=WGS84 +units=m");
	/* murd1 */ test_one<P>(4.897000, 52.371000, 333340.993642, 5839071.944597, "+proj=murd1 +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
	/* murd2 */ test_one<P>(4.897000, 52.371000, 317758.821713, 6759296.097305, "+proj=murd2 +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
	/* murd3 */ test_one<P>(4.897000, 52.371000, 331696.409000, 5839224.186916, "+proj=murd3 +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
	/* nell */ test_one<P>(4.897000, 52.371000, 454576.246081, 5355027.851999, "+proj=nell +ellps=WGS84 +units=m");
	/* nell_h */ test_one<P>(4.897000, 52.371000, 438979.742911, 5386970.539995, "+proj=nell_h +ellps=WGS84 +units=m");
	/* nicol */ test_one<P>(4.897000, 52.371000, 360493.071000, 5836451.532406, "+proj=nicol +ellps=WGS84 +units=m");
	/* nsper */ test_one<P>(4.897000, 52.371000, 0.521191, 7.919806, "+proj=nsper +ellps=WGS84 +units=m +a=10 +h=40000000");
	/* nzmg */ test_one<P>(174.783333, -36.850000, 2669448.884228, 6482177.102194, "+proj=nzmg +ellps=WGS84 +units=m");
	/* ocea */ test_one<P>(4.897000, 52.371000, 90368744555736.703000, -5887882801731.769500, "+proj=ocea +ellps=WGS84 +units=m  +lat_1=20n +lat_2=60n  +lon_1=1e +lon_2=30e");
	/* oea */ test_one<P>(4.897000, 52.371000, -1236967.015805, 3072499.235056, "+proj=oea +ellps=WGS84 +units=m +n=0.5 +m=0.5");
	/* omerc */ test_one<P>(4.897000, 52.371000, 1009705.329154, 5829437.254923, "+proj=omerc +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n  +lon_1=1e +lon_2=30e");
	/* ortel */ test_one<P>(4.897000, 52.371000, 360906.947408, 5829913.052335, "+proj=ortel +ellps=WGS84 +units=m");
	/* ortho */ test_one<P>(4.897000, 52.371000, 332422.874291, 5051361.531375, "+proj=ortho +ellps=WGS84 +units=m");
	/* pconic */ test_one<P>(4.897000, 52.371000, -34692.463986, 12802740.378536, "+proj=pconic +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
	/* poly */ test_one<P>(4.897000, 52.371000, 333274.269648, 5815908.957562, "+proj=poly +ellps=WGS84 +units=m");
	/* putp1 */ test_one<P>(4.897000, 52.371000, 375730.931178, 5523551.121434, "+proj=putp1 +ellps=WGS84 +units=m");
	/* putp2 */ test_one<P>(4.897000, 52.371000, 351480.997939, 5942668.547355, "+proj=putp2 +ellps=WGS84 +units=m");
	/* putp3 */ test_one<P>(4.897000, 52.371000, 287673.972013, 4651597.610600, "+proj=putp3 +ellps=WGS84 +units=m");
	/* putp3p */ test_one<P>(4.897000, 52.371000, 361313.008033, 4651597.610600, "+proj=putp3p +ellps=WGS84 +units=m");
	/* putp4p */ test_one<P>(4.897000, 52.371000, 351947.465829, 6330828.716145, "+proj=putp4p +ellps=WGS84 +units=m");
	/* putp5 */ test_one<P>(4.897000, 52.371000, 320544.316171, 5908383.682019, "+proj=putp5 +ellps=WGS84 +units=m");
	/* putp5p */ test_one<P>(4.897000, 52.371000, 436506.666600, 5908383.682019, "+proj=putp5p +ellps=WGS84 +units=m");
	/* putp6 */ test_one<P>(4.897000, 52.371000, 324931.055842, 5842588.644796, "+proj=putp6 +ellps=WGS84 +units=m");
	/* putp6p */ test_one<P>(4.897000, 52.371000, 338623.512107, 6396742.919679, "+proj=putp6p +ellps=WGS84 +units=m");
	/* qua_aut */ test_one<P>(4.897000, 52.371000, 370892.621714, 5629072.862494, "+proj=qua_aut +ellps=WGS84 +units=m");
	/* robin */ test_one<P>(4.897000, 52.371000, 394576.514058, 5571243.644839, "+proj=robin +ellps=WGS84 +units=m");
	/* rouss */ test_one<P>(4.897000, 52.371000, 412826.227669, 6248368.849775, "+proj=rouss +ellps=WGS84 +units=m");
	/* rpoly */ test_one<P>(4.897000, 52.371000, 332447.130797, 5841164.662431, "+proj=rpoly +ellps=WGS84 +units=m");
	/* sinu */ test_one<P>(4.897000, 52.371000, 333528.909809, 5804625.044313, "+proj=sinu +ellps=WGS84 +units=m");
	/* somerc */ test_one<P>(4.897000, 52.371000, 545131.546415, 6833623.829215, "+proj=somerc +ellps=WGS84 +units=m");
	/* stere */ test_one<P>(47.516667, -18.916667, 10864784.058711, -5015032.861234, "+proj=stere +ellps=WGS84 +units=m +lat_ts=30n");
	/* sterea */ test_one<P>(4.897000, 52.371000, 121590.388077, 487013.903377, "+proj=sterea +lat_0=52.15616055555555 +lon_0=5.38763888888889 +k=0.9999079 +x_0=155000 +y_0=463000 +ellps=bessel +units=m");
	/* tcc */ test_one<P>(4.897000, 52.371000, 332875.293370, 5841186.022551, "+proj=tcc +ellps=WGS84 +units=m");
	/* tcea */ test_one<P>(4.897000, 52.371000, 332422.874291, 5841186.022551, "+proj=tcea +ellps=WGS84 +units=m");
	/* tissot */ test_one<P>(4.897000, 52.371000, 431443.972539, 3808494.480735, "+proj=tissot +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
	/* tmerc */ test_one<P>(4.897000, 52.371000, 333425.492136, 5815921.814396, "+proj=tmerc +ellps=WGS84 +units=m");
	/* tpeqd */ test_one<P>(4.897000, 52.371000, 998886.128891, 873800.468721, "+proj=tpeqd +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n  +lon_1=0 +lon_2=30e");
	/* tpers */ test_one<P>(4.897000, 52.371000, -1172311.936260, 6263306.090352, "+proj=tpers +ellps=WGS84 +units=m +tilt=50 +azi=20 +h=40000000");
	/* ups */ test_one<P>(4.897000, 52.371000, 2369508.503532, -2312783.579527, "+proj=ups +ellps=WGS84 +units=m");
	/* urm5 */ test_one<P>(4.897000, 52.371000, 522185.854469, 5201544.371625, "+proj=urm5 +ellps=WGS84 +units=m +n=.3 +q=.3 +alpha=10");
	/* urmfps */ test_one<P>(4.897000, 52.371000, 439191.083465, 5919500.887257, "+proj=urmfps +ellps=WGS84 +units=m +n=0.50");
	/* utm */ test_one<P>(4.897000, 52.371000, 1037203.568847, 5831704.656129, "+proj=utm +ellps=WGS84 +units=m");
	/* vandg */ test_one<P>(4.897000, 52.371000, 489005.929978, 6431581.024949, "+proj=vandg +ellps=WGS84 +units=m");
	/* vandg2 */ test_one<P>(4.897000, 52.371000, 488953.592205, 6434578.861895, "+proj=vandg2 +ellps=WGS84 +units=m");
	/* vandg3 */ test_one<P>(4.897000, 52.371000, 489028.113123, 6430309.983824, "+proj=vandg3 +ellps=WGS84 +units=m");
	/* vandg4 */ test_one<P>(4.897000, 52.371000, 360804.549444, 5831531.435618, "+proj=vandg4 +ellps=WGS84 +units=m");
	/* vitk1 */ test_one<P>(4.897000, 52.371000, 338522.044182, 5839611.656064, "+proj=vitk1 +ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
	/* wag1 */ test_one<P>(4.897000, 52.371000, 348059.961742, 6344311.295111, "+proj=wag1 +ellps=WGS84 +units=m");
	/* wag2 */ test_one<P>(4.897000, 52.371000, 388567.174132, 6112322.636203, "+proj=wag2 +ellps=WGS84 +units=m");
	/* wag3 */ test_one<P>(4.897000, 52.371000, 447014.436776, 5829913.052335, "+proj=wag3 +ellps=WGS84 +units=m");
	/* wag4 */ test_one<P>(4.897000, 52.371000, 365021.547713, 6300040.998324, "+proj=wag4 +ellps=WGS84 +units=m");
	/* wag5 */ test_one<P>(4.897000, 52.371000, 379647.914735, 6771982.379506, "+proj=wag5 +ellps=WGS84 +units=m");
	/* wag6 */ test_one<P>(4.897000, 52.371000, 446107.907415, 5523551.121434, "+proj=wag6 +ellps=WGS84 +units=m");
	/* wag7 */ test_one<P>(4.897000, 52.371000, 366407.198644, 6169832.906560, "+proj=wag7 +ellps=WGS84 +units=m");
	/* weren */ test_one<P>(4.897000, 52.371000, 402668.037596, 7243190.025762, "+proj=weren +ellps=WGS84 +units=m");
	/* wink1 */ test_one<P>(4.897000, 52.371000, 438979.742911, 5829913.052335, "+proj=wink1 +ellps=WGS84 +units=m");
	/* wink2 */ test_one<P>(4.897000, 52.371000, 472810.645318, 6313461.757868, "+proj=wink2 +ellps=WGS84 +units=m");
	/* wintri */ test_one<P>(4.897000, 52.371000, 365568.851909, 5830576.163507, "+proj=wintri +ellps=WGS84 +units=m");
}


int test_main(int, char* [])
{
	//test_all<int[2]>();
	test_all<float[2]>();
	test_all<double[2]>();
	test_all<test_point>();
	//test_all<geometry::point_xy<int> >();
	test_all<geometry::point_xy<float> >();
	test_all<geometry::point_xy<double> >();
	test_all<geometry::point_xy<long double> >();

	return 0;
}


