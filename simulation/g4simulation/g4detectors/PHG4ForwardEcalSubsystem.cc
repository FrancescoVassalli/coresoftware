#include "PHG4ForwardEcalSubsystem.h"
#include "PHG4EICForwardEcalDetector.h"
#include "PHG4ForwardEcalDetector.h"
#include "PHG4ForwardEcalDisplayAction.h"
#include "PHG4ForwardEcalSteppingAction.h"

#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4DisplayAction.h>       // for PHG4DisplayAction
#include <g4main/PHG4SteppingAction.h>      // for PHG4SteppingAction
#include <g4main/PHG4Subsystem.h>           // for PHG4Subsystem

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>             // for PHIODataNode
#include <phool/PHNode.h>                   // for PHNode
#include <phool/PHNodeIterator.h>           // for PHNodeIterator
#include <phool/PHObject.h>                 // for PHObject
#include <phool/getClass.h>

#include <set>                              // for set
#include <sstream>

class PHG4Detector;

using namespace std;

//_______________________________________________________________________
PHG4ForwardEcalSubsystem::PHG4ForwardEcalSubsystem(const std::string& name, const int lyr)
  : PHG4Subsystem(name)
  , detector_(0)
  , steppingAction_(nullptr)
  , m_DisplayAction(nullptr)
  , active(1)
  , absorber_active(0)
  , blackhole(0)
  , detector_type(name)
  , mappingfile_("")
  , EICDetector(0)
{
}

//_______________________________________________________________________
PHG4ForwardEcalSubsystem::~PHG4ForwardEcalSubsystem()
{
  delete m_DisplayAction;
}

//_______________________________________________________________________
int PHG4ForwardEcalSubsystem::Init(PHCompositeNode* topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode* dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));

  // create display settings before detector
  m_DisplayAction = new PHG4ForwardEcalDisplayAction(Name());
  // create detector
  if (EICDetector)
    detector_ = new PHG4EICForwardEcalDetector(this, topNode, Name());
  else
    detector_ = new PHG4ForwardEcalDetector(this, topNode, Name());

  detector_->SetActive(active);
  detector_->SetAbsorberActive(absorber_active);
  detector_->BlackHole(blackhole);
  detector_->OverlapCheck(CheckOverlap());
  detector_->Verbosity(Verbosity());
  detector_->SetTowerMappingFile(mappingfile_);

  if (active)
  {
    set<string> nodes;

    // create hit output node
    ostringstream nodename;
    nodename << "G4HIT_" << detector_type;

    PHG4HitContainer* scintillator_hits = findNode::getClass<PHG4HitContainer>(topNode, nodename.str().c_str());
    if (!scintillator_hits)
    {
      scintillator_hits = new PHG4HitContainer(nodename.str());
      PHIODataNode<PHObject>* hitNode = new PHIODataNode<PHObject>(scintillator_hits, nodename.str().c_str(), "PHObject");
      dstNode->addNode(hitNode);
      nodes.insert(nodename.str());
    }

    ostringstream absnodename;
    absnodename << "G4HIT_ABSORBER_" << detector_type;

    PHG4HitContainer* absorber_hits = findNode::getClass<PHG4HitContainer>(topNode, absnodename.str().c_str());
    if (!absorber_hits)
    {
      absorber_hits = new PHG4HitContainer(absnodename.str());
      PHIODataNode<PHObject>* abshitNode = new PHIODataNode<PHObject>(absorber_hits, absnodename.str().c_str(), "PHObject");
      dstNode->addNode(abshitNode);
      nodes.insert(nodename.str());
    }

    // create stepping action
    steppingAction_ = new PHG4ForwardEcalSteppingAction(detector_);
  }

  return 0;
}

//_______________________________________________________________________
int PHG4ForwardEcalSubsystem::process_event(PHCompositeNode* topNode)
{
  // pass top node to stepping action so that it gets
  // relevant nodes needed internally
  if (steppingAction_)
  {
    steppingAction_->SetInterfacePointers(topNode);
  }
  return 0;
}

//_______________________________________________________________________
PHG4Detector* PHG4ForwardEcalSubsystem::GetDetector(void) const
{
  return detector_;
}
