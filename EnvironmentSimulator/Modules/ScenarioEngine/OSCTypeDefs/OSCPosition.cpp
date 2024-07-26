/*
 * esmini - Environment Simulator Minimalistic
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

#include "OSCPosition.hpp"

using namespace scenarioengine;

static void SetPositionModesGeneric(roadmanager::Position &position, double *z, OSCOrientation *orientation)
{
    position.SetModeDefault(roadmanager::Position::PosModeType::SET);
    position.SetModeDefault(roadmanager::Position::PosModeType::UPDATE);
#if 1
    position.SetModeBits(
        roadmanager::Position::PosModeType::INIT,
        ((z == nullptr || std::isnan(*z)) ? 0 : roadmanager::Position::PosMode::Z_ABS) |

            (orientation == nullptr || std::isnan(orientation->h_)
                 ? 0
                 : (orientation->type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE ? roadmanager::Position::PosMode::H_ABS
                                                                                                       : roadmanager::Position::PosMode::H_REL)) |
            (orientation == nullptr || std::isnan(orientation->p_)
                 ? 0
                 : (orientation->type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE ? roadmanager::Position::PosMode::P_ABS
                                                                                                       : roadmanager::Position::PosMode::P_REL)) |
            (orientation == nullptr || std::isnan(orientation->r_)
                 ? 0
                 : (orientation->type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE ? roadmanager::Position::PosMode::R_ABS
                                                                                                       : roadmanager::Position::PosMode::R_REL)));
#else
    position.SetModeBits(
        roadmanager::Position::PosModeType::INIT,
        ((z == nullptr || std::isnan(*z)) ? 0 : roadmanager::Position::PosMode::Z_ABS) |

            (orientation == nullptr || std::isnan(orientation->h_)
                 ? 0
                 : (orientation->type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE ? roadmanager::Position::PosMode::H_ABS
                                                                                                       : roadmanager::Position::PosMode::H_REL)) |
            (orientation == nullptr || std::isnan(orientation->p_)
                 ? 0
                 : (orientation->type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE ? roadmanager::Position::PosMode::P_ABS
                                                                                                       : roadmanager::Position::PosMode::P_REL)) |
            (orientation == nullptr || std::isnan(orientation->r_)
                 ? 0
                 : (orientation->type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE ? roadmanager::Position::PosMode::R_ABS
                                                                                                       : roadmanager::Position::PosMode::R_REL)));
#endif
}

OSCPositionWorld::OSCPositionWorld(double x, double y, double z, double h, double p, double r, OSCPosition *base_on_pos)
    : OSCPosition(PositionType::WORLD)
{
    if (base_on_pos != nullptr && base_on_pos->type_ == PositionType::WORLD)
    {
        this->position_ = *base_on_pos->GetRMPos();
    }

    OSCOrientation orientation(roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE, h, p, r);
    SetPositionModesGeneric(position_, &z, &orientation);
    position_.SetInertiaPosMode(x, y, z, h, p, r, position_.GetMode(roadmanager::Position::PosModeType::INIT));
}

OSCPositionLane::OSCPositionLane(int roadId, int laneId, double s, double offset, OSCOrientation orientation) : OSCPosition(PositionType::LANE)
{
    SetPositionModesGeneric(position_, nullptr, &orientation);

    // Set temporary position to find out road elevation, orientation and driving direction
    position_.SetLanePos(roadId, laneId, s, offset);

    if (orientation.type_ == roadmanager::Position::OrientationType::ORIENTATION_RELATIVE)
    {
        // Adjust heading to road direction also considering traffic rule (left/right hand traffic)
        if (position_.GetDrivingDirectionRelativeRoad() > 0)
        {
            position_.SetHeadingRelative(std::isnan(orientation.h_) ? 0.0 : orientation.h_);
        }
        else
        {
            position_.SetHeadingRelative(GetAngleSum(M_PI, std::isnan(orientation.h_) ? 0.0 : orientation.h_));
        }
        position_.SetPitchRelative(std::isnan(orientation.p_) ? 0.0 : orientation.p_);
        position_.SetRollRelative(std::isnan(orientation.r_) ? 0.0 : orientation.r_);
    }
    else if (orientation.type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE)
    {
        position_.SetHeading(std::isnan(orientation.h_) ? 0.0 : orientation.h_);
        position_.SetPitch(std::isnan(orientation.p_) ? 0.0 : orientation.p_);
        position_.SetRoll(std::isnan(orientation.r_) ? 0.0 : orientation.r_);
    }
    else
    {
        LOG("Unexpected orientation type: %d", orientation.type_);
    }

    position_.SetLanePosMode(roadId, laneId, s, offset, position_.GetMode(roadmanager::Position::PosModeType::UPDATE));
}

OSCPositionRoad::OSCPositionRoad(int roadId, double s, double t, OSCOrientation orientation) : OSCPosition(PositionType::ROAD)
{
    if (position_.GetRoadById(roadId) == nullptr)
    {
        LOG_AND_QUIT("Reffered road ID %d not available in road network", roadId);
    }

    SetPositionModesGeneric(position_, nullptr, &orientation);

    // Set temporary position to find out road elevation, orientation and driving direction
    position_.SetTrackPos(roadId, s, t);

    if (orientation.type_ == roadmanager::Position::OrientationType::ORIENTATION_RELATIVE)
    {
        position_.SetHeadingRelative(std::isnan(orientation.h_) ? 0.0 : orientation.h_);
        position_.SetPitchRelative(std::isnan(orientation.p_) ? 0.0 : orientation.p_);
        position_.SetRollRelative(std::isnan(orientation.r_) ? 0.0 : orientation.r_);
    }
    else if (orientation.type_ == roadmanager::Position::OrientationType::ORIENTATION_ABSOLUTE)
    {
        position_.SetHeading(std::isnan(orientation.h_) ? 0.0 : orientation.h_);
        position_.SetPitch(std::isnan(orientation.p_) ? 0.0 : orientation.p_);
        position_.SetRoll(std::isnan(orientation.r_) ? 0.0 : orientation.r_);
    }
    else
    {
        LOG("Unexpected orientation type: %d", orientation.type_);
    }

    position_.SetTrackPosMode(roadId, s, t, position_.GetMode(roadmanager::Position::PosModeType::UPDATE));
}

OSCPositionRelativeObject::OSCPositionRelativeObject(Object *object, double dx, double dy, double dz, OSCOrientation orientation)
    : OSCPosition(PositionType::RELATIVE_OBJECT),
      object_(object)
{
    SetPositionModesGeneric(position_, nullptr, &orientation);

    position_.relative_.dh = std::isnan(orientation.h_) ? 0.0 : orientation.h_;
    position_.relative_.dp = std::isnan(orientation.p_) ? 0.0 : orientation.p_;
    position_.relative_.dr = std::isnan(orientation.r_) ? 0.0 : orientation.r_;

    position_.relative_.dx = dx;
    position_.relative_.dy = dy;
    position_.relative_.dz = std::isnan(dz) ? 0.0 : dz;

    position_.SetRelativePosition(&object->pos_, roadmanager::Position::PositionType::RELATIVE_OBJECT);
}

void OSCPositionRelativeObject::Print()
{
    object_->pos_.Print();
}

OSCPositionRelativeWorld::OSCPositionRelativeWorld(Object *object, double dx, double dy, double dz, OSCOrientation orientation)
    : OSCPosition(PositionType::RELATIVE_WORLD),
      object_(object)
{
    SetPositionModesGeneric(position_, nullptr, &orientation);

    position_.relative_.dx = dx;
    position_.relative_.dy = dy;
    position_.relative_.dz = std::isnan(dz) ? 0.0 : dz;

    position_.relative_.dh = std::isnan(orientation.h_) ? 0.0 : orientation.h_;
    position_.relative_.dp = std::isnan(orientation.p_) ? 0.0 : orientation.p_;
    position_.relative_.dr = std::isnan(orientation.r_) ? 0.0 : orientation.r_;

    position_.SetRelativePosition(&object->pos_, roadmanager::Position::PositionType::RELATIVE_WORLD);
}

void OSCPositionRelativeWorld::Print()
{
    object_->pos_.Print();
}

OSCPositionRelativeLane::OSCPositionRelativeLane(Object                              *object,
                                                 int                                  dLane,
                                                 double                               ds,
                                                 double                               offset,
                                                 OSCOrientation                       orientation,
                                                 roadmanager::Position::DirectionMode direction_mode)
    : OSCPosition(PositionType::RELATIVE_LANE),
      object_(object)
{
    SetPositionModesGeneric(position_, nullptr, &orientation);

    position_.relative_.dLane  = dLane;
    position_.relative_.ds     = ds;
    position_.relative_.offset = offset;
    position_.SetDirectionMode(direction_mode);

    position_.relative_.dh = std::isnan(orientation.h_) ? 0.0 : orientation.h_;
    position_.relative_.dp = std::isnan(orientation.p_) ? 0.0 : orientation.p_;
    position_.relative_.dr = std::isnan(orientation.r_) ? 0.0 : orientation.r_;

    position_.SetRelativePosition(&object->pos_, roadmanager::Position::PositionType::RELATIVE_LANE);
}

void OSCPositionRelativeLane::Print()
{
    object_->pos_.Print();
}

OSCPositionRelativeRoad::OSCPositionRelativeRoad(Object *object, double ds, double dt, OSCOrientation orientation)
    : OSCPosition(PositionType::RELATIVE_ROAD),
      object_(object)
{
    SetPositionModesGeneric(position_, nullptr, &orientation);

    position_.relative_.ds = ds;
    position_.relative_.dt = dt;

    position_.relative_.dh = std::isnan(orientation.h_) ? 0.0 : orientation.h_;
    position_.relative_.dp = std::isnan(orientation.p_) ? 0.0 : orientation.p_;
    position_.relative_.dr = std::isnan(orientation.r_) ? 0.0 : orientation.r_;

    position_.SetRelativePosition(&object->pos_, roadmanager::Position::PositionType::RELATIVE_ROAD);
}

void OSCPositionRelativeRoad::Print()
{
    object_->pos_.Print();
}

OSCPositionRoute::OSCPositionRoute(roadmanager::Route *route, double s, int laneId, double laneOffset)
{
    (void)s;
    (void)laneId;
    (void)laneOffset;
    position_.SetRoute(route);
}

void OSCPositionRoute::SetRouteRefLaneCoord(roadmanager::Route *route, double pathS, int laneId, double laneOffset, OSCOrientation *orientation)
{
    position_.SetRouteLanePosition(route, pathS, laneId, laneOffset);

    // Adjust heading to road direction also considering traffic rule (left/right hand traffic)
    if (position_.GetDrivingDirectionRelativeRoad() < 0)
    {
        position_.SetHeadingRelative(GetAngleSum(M_PI, std::isnan(orientation->h_) ? 0.0 : orientation->h_));
        position_.SetPitchRelative(std::isnan(orientation->p_) ? 0.0 : -orientation->p_);
        position_.SetRollRelative(std::isnan(orientation->r_) ? 0.0 : -orientation->r_);
    }
    else
    {
        position_.SetHeadingRelative(std::isnan(orientation->h_) ? 0.0 : orientation->h_);
        position_.SetPitchRelative(std::isnan(orientation->p_) ? 0.0 : orientation->p_);
        position_.SetRollRelative(std::isnan(orientation->r_) ? 0.0 : orientation->r_);
    }
}

void OSCPositionRoute::SetRouteRefLaneCoord(roadmanager::Route *route, double pathS, int laneId, double laneOffset)
{
    position_.SetRouteLanePosition(route, pathS, laneId, laneOffset);
}

OSCPositionTrajectory::OSCPositionTrajectory(roadmanager::RMTrajectory *traj, double s, double t, OSCOrientation orientation)
{
    (void)orientation;
    (void)t;

    // do initial and temporary evaluation of the trajectory
    // it will be recalculated when related action is triggered
    // so that relative positions will be evaluated correctly
    traj->shape_->CalculatePolyLine();

    position_.SetTrajectory(traj);
    position_.SetTrajectoryS(s);
}
