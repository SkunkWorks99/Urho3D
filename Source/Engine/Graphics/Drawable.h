//
// Copyright (c) 2008-2013 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "BoundingBox.h"
#include "Component.h"
#include "GraphicsDefs.h"

namespace Urho3D
{

static const unsigned DRAWABLE_GEOMETRY = 0x1;
static const unsigned DRAWABLE_LIGHT = 0x2;
static const unsigned DRAWABLE_ZONE = 0x4;
static const unsigned DRAWABLE_ANY = 0xff;
static const unsigned DEFAULT_VIEWMASK = M_MAX_UNSIGNED;
static const unsigned DEFAULT_LIGHTMASK = M_MAX_UNSIGNED;
static const unsigned DEFAULT_SHADOWMASK = M_MAX_UNSIGNED;
static const unsigned DEFAULT_ZONEMASK = M_MAX_UNSIGNED;
static const int MAX_VERTEX_LIGHTS = 4;
static const float ANIMATION_LOD_BASESCALE = 2500.0f;

class Camera;
class Geometry;
class Light;
class Material;
class OcclusionBuffer;
class Octant;
class RayOctreeQuery;
class Zone;
struct RayQueryResult;
struct WorkItem;

/// Geometry update type.
enum UpdateGeometryType
{
    UPDATE_NONE = 0,
    UPDATE_MAIN_THREAD,
    UPDATE_WORKER_THREAD
};

/// Rendering frame update parameters.
struct FrameInfo
{
    /// Frame number.
    unsigned frameNumber_;
    /// Time elapsed since last frame.
    float timeStep_;
    /// Viewport size.
    IntVector2 viewSize_;
    /// Camera being used.
    Camera* camera_;
};

/// Source data for a 3D geometry draw call.
struct SourceBatch
{
    /// Construct with defaults.
    SourceBatch();
    /// Destruct.
    ~SourceBatch();
    
    /// Distance from camera.
    float distance_;
    /// Geometry.
    Geometry* geometry_;
    /// Material.
    SharedPtr<Material> material_;
    /// World transform(s). For a skinned model, these are the bone transforms.
    const Matrix3x4* worldTransform_;
    /// Number of world transforms.
    unsigned numWorldTransforms_;
    /// %Geometry type.
    GeometryType geometryType_;
    /// Override view transform flag.
    bool overrideView_;
};

/// Base class for visible components.
class URHO3D_API Drawable : public Component
{
    OBJECT(Drawable);
    
    friend class Octant;
    friend class Octree;
    friend void UpdateDrawablesWork(const WorkItem* item, unsigned threadIndex);
    
public:
    /// Construct.
    Drawable(Context* context, unsigned char drawableFlags);
    /// Destruct.
    virtual ~Drawable();
    /// Register object attributes. Drawable must be registered first.
    static void RegisterObject(Context* context);
    
    /// Handle enabled/disabled state change.
    virtual void OnSetEnabled();
    /// Process octree raycast. May be called from a worker thread.
    virtual void ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results);
    /// Update before octree reinsertion. Is called from a worker thread.
    virtual void Update(const FrameInfo& frame) {}
    /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
    virtual void UpdateBatches(const FrameInfo& frame);
    /// Prepare geometry for rendering.
    virtual void UpdateGeometry(const FrameInfo& frame) {}
    /// Return whether a geometry update is necessary, and if it can happen in a worker thread.
    virtual UpdateGeometryType GetUpdateGeometryType() { return UPDATE_NONE; }
    /// Return the geometry for a specific LOD level.
    virtual Geometry* GetLodGeometry(unsigned batchIndex, unsigned level);
    /// Return number of occlusion geometry triangles.
    virtual unsigned GetNumOccluderTriangles() { return 0; }
    /// Draw to occlusion buffer. Return true if did not run out of triangles.
    virtual bool DrawOcclusion(OcclusionBuffer* buffer) { return true; }
    /// Visualize the component as debug geometry.
    virtual void DrawDebugGeometry(DebugRenderer* debug, bool depthTest);
    
    /// Set draw distance.
    void SetDrawDistance(float distance);
    /// Set shadow draw distance.
    void SetShadowDistance(float distance);
    /// Set LOD bias.
    void SetLodBias(float bias);
    /// Set view mask. Is and'ed with camera's view mask to see if the object should be rendered.
    void SetViewMask(unsigned mask);
    /// Set light mask. Is and'ed with light's and zone's light mask to see if the object should be lit.
    void SetLightMask(unsigned mask);
    /// Set shadow mask. Is and'ed with light's light mask and zone's shadow mask to see if the object should be rendered to a shadow map.
    void SetShadowMask(unsigned mask);
    /// Set zone mask. Is and'ed with zone's zone mask to see if the object should belong to the zone.
    void SetZoneMask(unsigned mask);
    /// Set maximum number of per-pixel lights. Default 0 is unlimited.
    void SetMaxLights(unsigned num);
    /// Set shadowcaster flag.
    void SetCastShadows(bool enable);
    /// Set occlusion flag.
    void SetOccluder(bool enable);
    /// Set occludee flag.
    void SetOccludee(bool enable);
    /// Mark for update and octree reinsertion. Update is automatically queued when the drawable's scene node moves or changes scale.
    void MarkForUpdate();
    
    /// Return local space bounding box. May not be applicable or properly updated on all drawables.
    const BoundingBox& GetBoundingBox() const { return boundingBox_; }
    /// Return world-space bounding box.
    const BoundingBox& GetWorldBoundingBox();
    /// Return drawable flags.
    unsigned char GetDrawableFlags() const { return drawableFlags_; }
    /// Return draw distance.
    float GetDrawDistance() const { return drawDistance_; }
    /// Return shadow draw distance.
    float GetShadowDistance() const { return shadowDistance_; }
    /// Return LOD bias.
    float GetLodBias() const { return lodBias_; }
    /// Return view mask.
    unsigned GetViewMask() const { return viewMask_; }
    /// Return light mask.
    unsigned GetLightMask() const { return lightMask_; }
    /// Return shadow mask.
    unsigned GetShadowMask() const { return shadowMask_; }
    /// Return zone mask.
    unsigned GetZoneMask() const { return zoneMask_; }
    /// Return maximum number of per-pixel lights.
    unsigned GetMaxLights() const { return maxLights_; }
    /// Return shadowcaster flag.
    bool GetCastShadows() const { return castShadows_; }
    /// Return occluder flag.
    bool IsOccluder() const { return occluder_; }
    /// Return occludee flag.
    bool IsOccludee() const { return occludee_; }
    /// Return draw call source data.
    const Vector<SourceBatch>& GetBatches() const { return batches_; }
    
    /// Set new zone. Zone assignment may optionally be temporary, meaning it needs to be re-evaluated on the next frame.
    void SetZone(Zone* zone, bool temporary = false);
    /// Set sorting value.
    void SetSortValue(float value);
    /// Set view-space depth bounds.
    void SetMinMaxZ(float minZ, float maxZ);
    /// Mark in view (either the main camera, or a shadow camera view) this frame.
    void MarkInView(const FrameInfo& frame, bool mainView = true);
    /// Sort and limit per-pixel lights to maximum allowed. Convert extra lights into vertex lights.
    void LimitLights();
    /// Sort and limit per-vertex lights to maximum allowed.
    void LimitVertexLights();
    /// Set base pass flag for a batch.
    void SetBasePass(unsigned batchIndex) { basePassFlags_ |= (1 << batchIndex); }
    /// Return octree octant.
    Octant* GetOctant() const { return octant_; }
    /// Return current zone.
    Zone* GetZone() const { return zone_; }
    /// Return previous zone.
    Zone* GetLastZone() const { return lastZone_; }
    /// Return if zone assignment needs re-evaluation.
    bool IsZoneDirty() const { return zoneDirty_; }
    /// Return distance from camera.
    float GetDistance() const { return distance_; }
    /// Return LOD scaled distance from camera.
    float GetLodDistance() const { return lodDistance_; }
    /// Return sorting value.
    float GetSortValue() const { return sortValue_; }
    /// Return whether is in view this frame.
    bool IsInView(unsigned frameNumber) const { return viewFrameNumber_ == frameNumber; }
    /// Return whether is visible in a specific view this frame.
    bool IsInView(const FrameInfo& frame, bool mainView = true) const { return viewFrameNumber_ == frame.frameNumber_ && viewFrame_ == &frame && (!mainView || viewCamera_ == frame.camera_); }
    /// Return whether has a base pass.
    bool HasBasePass(unsigned batchIndex) const { return (basePassFlags_ & (1 << batchIndex)) != 0; }
    /// Return per-pixel lights.
    const PODVector<Light*>& GetLights() const { return lights_; }
    /// Return per-vertex lights.
    const PODVector<Light*>& GetVertexLights() const { return vertexLights_; }
    /// Return the first added per-pixel light.
    Light* GetFirstLight() const { return firstLight_; }
    /// Return the minimum view-space depth.
    float GetMinZ() const { return minZ_; }
    /// Return the maximum view-space depth.
    float GetMaxZ() const { return maxZ_; }
    
    // Clear the frame's light list.
    void ClearLights()
    {
        basePassFlags_ = 0;
        firstLight_ = 0;
        lights_.Clear();
        vertexLights_.Clear();
    }

    // Add a per-pixel light affecting the object this frame.
    void AddLight(Light* light)
    {
        if (lights_.Empty())
            firstLight_ = light;
        lights_.Push(light);
    }

    // Add a per-vertex light affecting the object this frame.
    void AddVertexLight(Light* light)
    {
        vertexLights_.Push(light);
    }

protected:
    /// Handle node being assigned.
    virtual void OnNodeSet(Node* node);
    /// Handle node transform being dirtied.
    virtual void OnMarkedDirty(Node* node);
    /// Recalculate the world-space bounding box.
    virtual void OnWorldBoundingBoxUpdate() = 0;
    /// Add to octree.
    void AddToOctree();
    /// Remove from octree.
    void RemoveFromOctree();
    /// Move into another octree octant.
    void SetOctant(Octant* octant) { octant_ = octant; }
    
    /// World-space bounding box.
    BoundingBox worldBoundingBox_;
    /// Local-space bounding box.
    BoundingBox boundingBox_;
    /// Draw call source data.
    Vector<SourceBatch> batches_;
    /// Drawable flags.
    unsigned char drawableFlags_;
    /// Bounding box dirty flag.
    bool worldBoundingBoxDirty_;
    /// Shadowcaster flag.
    bool castShadows_;
    /// Occluder flag.
    bool occluder_;
    /// Occludee flag.
    bool occludee_;
    /// Octree update queued flag.
    bool updateQueued_;
    /// View mask.
    unsigned viewMask_;
    /// Light mask.
    unsigned lightMask_;
    /// Shadow mask.
    unsigned shadowMask_;
    /// Zone mask.
    unsigned zoneMask_;
    /// Last visible frame number.
    unsigned viewFrameNumber_;
    /// Current distance to camera.
    float distance_;
    /// LOD scaled distance.
    float lodDistance_;
    /// Draw distance.
    float drawDistance_;
    /// Shadow distance.
    float shadowDistance_;
    /// Current sort value.
    float sortValue_;
    /// Current minimum view space depth.
    float minZ_;
    /// Current maximum view space depth.
    float maxZ_;
    /// LOD bias.
    float lodBias_;
    /// Base pass flags.
    unsigned basePassFlags_;
    /// Maximum lights.
    unsigned maxLights_;
    /// Octree octant.
    Octant* octant_;
    /// First per-pixel light added this frame.
    Light* firstLight_;
    /// Per-pixel lights affecting this drawable.
    PODVector<Light*> lights_;
    /// Per-vertex lights affecting this drawable.
    PODVector<Light*> vertexLights_;
    /// Current zone.
    Zone* zone_;
    /// Previous zone.
    Zone* lastZone_;
    /// Last view's frameinfo. Not safe to dereference.
    const FrameInfo* viewFrame_;
    /// Last view's camera. Not safe to dereference.
    Camera* viewCamera_;
    /// Zone assignment dirty flag.
    bool zoneDirty_;
};

inline bool CompareDrawables(Drawable* lhs, Drawable* rhs)
{
    return lhs->GetSortValue() < rhs->GetSortValue();
}

}
