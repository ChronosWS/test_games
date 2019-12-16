#pragma once

#include "components.h"
#include "ecs/ecs.h"
#include "components/common/transform_component.h"
#include "components/rendering/rectangle_component.h"
#include "components/rendering/triangle_component.h"

// Add new game types here.
inline ecs::ComponentStorage<
        DestinationComponent,
        RectangleComponent,
        TransformComponent,
        TriangleComponent> kECS;
