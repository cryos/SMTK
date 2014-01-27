#include "smtk/cgm/Engines.h"

#include "smtk/cgm/CAUUID.h"

#include "DLIList.hpp"
#include "FacetModifyEngine.hpp"
#include "GeometryHealerTool.hpp"
#include "GeometryQueryTool.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryHealerEngine.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryModifyEngine.hpp"
#include "InitCGMA.hpp"

#ifdef HAVE_OCC
#  include "OCCModifyEngine.hpp"
#endif

#include <algorithm>

namespace cgmsmtk {
  namespace cgm {

static bool cgmaInitialized = false;
/* We keep these around because there are no headers for the Cubit(ACIS)
 * engine, even when building against Cubit. There are also no virtual
 * methods to identify which modeler is associated with a Modify or Healer
 * engine pointer. So, the only way to obtain the cubit Modify and Healer
 * engines is to initialize CGMA with Cubit/ACIS as the default engine and,
 * if successful, save pointers the resulting engines.
 *
 * Ugly ugly ugly.
 */
static bool originalEnginesAreACIS = false;
static GeometryQueryEngine* originalQueryEngine = NULL;
static GeometryModifyEngine* originalModifyEngine = NULL;
static GeometryHealerEngine* originalHealerEngine = NULL;

bool Engines::areInitialized()
{
  return cgmaInitialized;
}

bool Engines::isInitialized(const std::string& engine, const std::vector<std::string>& args)
{
  // Always try to initialize with ACIS first for reason discussed above.
  if (!cgmaInitialized && !originalQueryEngine && engine != "ACIS" && engine != "Cubit")
    {
    if (Engines::isInitialized("ACIS", args))
      {
      originalEnginesAreACIS = true;
      }
    cgmaInitialized = originalQueryEngine ? true : false;
    // OK, we got ACIS defaults, now make the user-requested engine default
    cgmaInitialized = Engines::setDefault(engine);
    return cgmaInitialized;
    }

  // OK, either the user is asking for ACIS or we don't have it anyway...
  std::vector<const char*> targs; // translated arguments
  targs.push_back("smtk"); // input \a args should not provide program name.
  std::vector<std::string>::const_iterator ait;
  for (ait = args.begin(); ait != args.end(); ++ait)
    targs.push_back(ait->c_str());

#if CGM_MAJOR_VERSION >= 14
  std::vector<CubitString> cargs(targs.begin(), targs.end());
  CGMApp::instance()->startup(cargs);
#else
  CubitObserver::init_static_observers();
  CGMApp::instance()->startup(targs.size(), const_cast<char**>(&targs[0]));
#endif

  cgmsmtk::cgm::CAUUID::registerWithAttributeManager();
  CubitStatus s = InitCGMA::initialize_cgma(
    engine.empty() ? "OCC" : engine.c_str());
  cgmaInitialized = (s == CUBIT_SUCCESS ? true : false);
  if (!originalQueryEngine)
    {
    originalQueryEngine = GeometryQueryTool::instance()->get_gqe();
    originalModifyEngine = GeometryModifyTool::instance()->get_gme();
    }
  return cgmaInitialized;
}

bool Engines::setDefault(const std::string& engine)
{
  // If we've never called init, do some extra stuff:
  if (!Engines::areInitialized())
    {
    return Engines::isInitialized(engine);
    }
  // Otherwise, see if we can create the appropriate engines
  // and set them in the default query/modify tool.

  // Downcase the engine string. Some facet engines return "facet"
  // for their modeler type, but the engine itself is named "FACET".
  // Grrr....
  std::string engineLower = engine;
  std::transform(engineLower.begin(), engineLower.end(), engineLower.begin(),
    std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("")));

  bool defaultChanged = false;
  DLIList<GeometryModifyEngine*> gmes;
  GeometryModifyEngine* gme;
  GeometryModifyTool* gmt = NULL;
  DLIList<GeometryQueryEngine*> gqes;
  GeometryQueryEngine* gqe;
  GeometryQueryTool* gqt = NULL;
  //DLIList<GeometryHealerEngine*> ghes;
  //GeometryHealerEngine* ghe;
  //GeometryHealerTool* ght = NULL;
  // GeometryFeatureTool does not provide list of engines

  GeometryQueryTool::instance()->get_gqe_list(gqes);
  for (int i = 0; i < gqes.size(); ++i)
    {
    gqe = gqes.get_and_step();
    std::string modeler = gqe->modeler_type();
    std::transform(modeler.begin(), modeler.end(), modeler.begin(),
      std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("")));
    if (modeler == engineLower)
      {
      GeometryQueryTool::instance()->set_default_gqe(gqe);
      gqt = GeometryQueryTool::instance();
      break;
      }
    }

  /* GeometryModifyEngine does not provide modeler_type().
  GeometryModifyTool::instance()->get_gme_list(gmes);
  for (int i = 0; i < gmes.size(); ++i)
    {
    gme = gmes.get_and_step();
    std::string modeler = gme->modeler_type();
    if (modeler == engineLower)
      {
      gmt = GeometryModifyTool::instance(gme);
      break;
      }
    }
    */
  if (engine == "FACET")
    gme = FacetModifyEngine::instance();
#ifdef HAVE_OCC
  else if (engine == "OCC")
    gme = OCCModifyEngine::instance();
#endif
  else if ((engine == "ACIS" || engine == "Cubit") && originalEnginesAreACIS)
    gme = originalModifyEngine;
  else
    gme = NULL;
  if (gme)
    {
    gmt = GeometryModifyTool::instance(gme);
    }

  /* GeometryHealerEngine does not provide modeler_type().
   * Nor does GeometryHealerTool provide an instance() method
   * or a get_ghe() method.
   *
  GeometryHealerTool::instance()->get_ghe_list(ghes);
  for (int i = 0; i < ghes.size(); ++i)
    {
    ghe = ghes.get_and_step();
    std::string modeler = ghe->modeler_type();
    if (modeler == engineLower)
      {
      ght = GeometryHealerTool::instance(ghe);
      break;
      }
    }
    */

  // For now, we cannot rely on anyone providing a healer.
  defaultChanged = (gmt && gqt ? true : false);
  return defaultChanged;
}

bool Engines::shutdown()
{
  InitCGMA::deinitialize_cgma();
  cgmaInitialized = false;
  originalQueryEngine = NULL;
  originalModifyEngine = NULL;
  originalHealerEngine = NULL;
  return true;
}

  } // namespace cgm
} // namespace cgmsmtk
