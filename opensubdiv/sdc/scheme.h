//
//   Copyright 2014 DreamWorks Animation LLC.
//
//   Licensed under the Apache License, Version 2.0 (the "Apache License")
//   with the following modification; you may not use this file except in
//   compliance with the Apache License and the following modification to it:
//   Section 6. Trademarks. is deleted and replaced with:
//
//   6. Trademarks. This License does not grant permission to use the trade
//      names, trademarks, service marks, or product names of the Licensor
//      and its affiliates, except as required to comply with Section 4(c) of
//      the License and to reproduce the content of the NOTICE file.
//
//   You may obtain a copy of the Apache License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the Apache License with the above modification is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
//   KIND, either express or implied. See the Apache License for the specific
//   language governing permissions and limitations under the Apache License.
//
#ifndef SDC_SCHEME_H
#define SDC_SCHEME_H

#include "../version.h"

#include "../sdc/type.h"
#include "../sdc/options.h"
#include "../sdc/crease.h"

#include <cassert>
#include <cstdlib>
#include <vector>

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Sdc {

//
//  Scheme is a class template which provides all implementation for the subdivision schemes
//  supported by OpenSubdiv through specializations of the methods of each.  An instance of
//  Scheme<SCHEME_TYPE> includes a set of Options that will dictate the variable aspects of
//  its behavior.
//
//  The primary purpose of Scheme is to provide the mask weights for vertices generated by
//  subdivision.  Methods to determine the masks are given topological neighborhoods from which
//  to compute the appropriate weights for neighboring components.  While these neighborhoods
//  may require sharpness values for creasing, the computation of subdivided crease values is
//  independent of the scheme type and is also made available through the Crease class.
//
//  Mask queries are assisted by two utility classes -- a Neighborhood class defining the set
//  of relevant data in the topological neighborhood of the vertex being subdivided, and a Mask
//  class into which the associated mask weights will be stored.  Depending on where and how
//  these queries are used, more or less information may be available.  See the details of the
//  Neighborhood classes as appropriate initialization of them is critical.  It is generally
//  best to initialize them with what data is known and accessible for immediate and efficient
//  retrieval, but subclasses can be created to gather it lazily if desired.
//

template <Type SCHEME_TYPE>
class Scheme {

public:

    Scheme() : _options() { }

    Scheme(Options const& options) : _options(options) { }

    ~Scheme() { }

    Options GetOptions() const { return _options; }
    void    SetOptions(const Options& newOptions) { _options = newOptions; }

    //
    //  Face-vertex masks - trivial for all current schemes:
    //
    template <typename FACE, typename MASK>
    void ComputeFaceVertexMask(FACE const& faceNeighborhood, MASK& faceVertexMask) const;

    //
    //  Edge-vertex masks:
    //      If known, the Rule for the edge and/or the derived vertex can be specified to
    //  accelerate the computation (though the Rule for the parent is trivially determined).
    //  In particular, knowing the child rule can avoid the need to subdivide the sharpness
    //  of the edge to see if it is a transitional crease that warrants fractional blending.
    //
    //  Whether to use the "Rules" in this interface is really debatable -- the parent Rule
    //  is really based on the edge and its sharpness, while the child Rule is technically
    //  based on the neighborhood of the child vertex, but it can be deduced from the two
    //  child edges' sharpness.  So the Crease methods used to compute these rules differ
    //  from those for the vertex-vertex mask.  Perhaps a simple pair of new methods for
    //  Crease should be added specific to the edge-vertex case, i.e. one that takes a
    //  single sharpness (for the parent rule) and one that takes a pair (for the child).
    //
    template <typename EDGE, typename MASK>
    void ComputeEdgeVertexMask(EDGE const& edgeNeighborhood, MASK& edgeVertexMask,
                               Crease::Rule parentRule = Crease::RULE_UNKNOWN,
                               Crease::Rule childRule = Crease::RULE_UNKNOWN) const;

    //
    //  Vertex-vertex masks:
    //      If known, a single Rule or pair of Rules can be specified (indicating a crease
    //  transition) to accelerate the computation.  Either no Rules, the first, or both should
    //  be specified.  Specification of only the first Rule implies it to be true for both
    //  (wish the compiler would allow such default value specification), i.e. no transition.
    //  The case of knowing the parent Rule but deferring determination of the child Rule to
    //  this method is not supported.
    //
    template <typename VERTEX, typename MASK>
    void ComputeVertexVertexMask(VERTEX const& vertexNeighborhood, MASK& vertexVertexMask,
                                    Crease::Rule parentRule = Crease::RULE_UNKNOWN,
                                    Crease::Rule childRule = Crease::RULE_UNKNOWN) const;

    //
    //  IN PROGRESS -- NOT YET FULLY FUNCTIONAL...
    //
    //  Masks for limit points and tangents -- note that these require the vertex be
    //  suitably isolated such that its limit is well-defined.
    //
    //  These are stubs that are still being completed.  The position masks are now
    //  supported but tangent masks need work.
    //
    template <typename VERTEX, typename MASK>
    void ComputeVertexLimitMask(VERTEX const& vertexNeighborhood, MASK& positionMask) const;

    template <typename VERTEX, typename MASK>
    void ComputeVertexLimitMask(VERTEX const& vertexNeighborhood, MASK& positionMask,
                                                                  MASK& tangent1Mask,
                                                                  MASK& tangent2Mask) const;
protected:
    //
    //  Supporting internal methods -- optionally implemented, depending on specialization:
    //

    //
    //  Subdivision/refinement masks -- two for edge-vertices and three for vertex-vertices:
    //
    template <typename EDGE, typename MASK>
    void assignCreaseMaskForEdge(EDGE const& edge, MASK& mask) const;
    template <typename EDGE, typename MASK>
    void assignSmoothMaskForEdge(EDGE const& edge, MASK& mask) const;

    template <typename VERTEX, typename MASK>
    void assignCornerMaskForVertex(VERTEX const& edge, MASK& mask) const;
    template <typename VERTEX, typename MASK>
    void assignCreaseMaskForVertex(VERTEX const& edge, MASK& mask, float const sharpness[]) const;
    template <typename VERTEX, typename MASK>
    void assignSmoothMaskForVertex(VERTEX const& edge, MASK& mask) const;

    //
    //  Limit masks for position and tangents -- boundary and interior cases for both:
    //
    template <typename VERTEX, typename MASK>
    void assignBoundaryLimitMask(VERTEX const& vertex, MASK& pos) const;
    template <typename VERTEX, typename MASK>
    void assignInteriorLimitMask(VERTEX const& vertex, MASK& pos) const;

    template <typename VERTEX, typename MASK>
    void assignBoundaryLimitTangentMasks(VERTEX const& vertex, MASK& tan1, MASK& tan2) const;
    template <typename VERTEX, typename MASK>
    void assignInteriorLimitTangentMasks(VERTEX const& vertex, MASK& tan1, MASK& tan2) const;

private:
    Options _options;

protected:
    //
    //  Internal implementation support:
    //
    //  We need a local "mask" class to be declared locally within the vertex-vertex mask query
    //  to hold one of the two possible mask required and to combine the local mask with the mask
    //  the caller provides.  It has been parameterized by <WEIGHT> so that a version compatible
    //  with the callers mask class is created.
    //
    template <typename WEIGHT>
    class LocalMask {

    public:
        typedef WEIGHT Weight;

    public:
        LocalMask(Weight* v, Weight* e, Weight* f) : _vWeights(v), _eWeights(e), _fWeights(f) { }
        ~LocalMask() { }

    public:

        //
        //  Methods required for general mask assignments and queries:
        //
        int GetNumVertexWeights() const { return _vCount; }
        int GetNumEdgeWeights()   const { return _eCount; }
        int GetNumFaceWeights()   const { return _fCount; }

        void SetNumVertexWeights(int count) { _vCount = count; }
        void SetNumEdgeWeights(  int count) { _eCount = count; }
        void SetNumFaceWeights(  int count) { _fCount = count; }

        Weight const& VertexWeight(int index) const { return _vWeights[index]; }
        Weight const& EdgeWeight(  int index) const { return _eWeights[index]; }
        Weight const& FaceWeight(  int index) const { return _fWeights[index]; }

        Weight& VertexWeight(int index) { return _vWeights[index]; }
        Weight& EdgeWeight(  int index) { return _eWeights[index]; }
        Weight& FaceWeight(  int index) { return _fWeights[index]; }

        bool AreFaceWeightsForFaceCenters() const  { return _fWeightsForCenters; }
        void SetFaceWeightsForFaceCenters(bool on) { _fWeightsForCenters = on; }

    public:

        //
        //  Additional methods -- mainly the blending method for vertex-vertex masks:
        //
        template <typename USER_MASK>
        inline void
        CombineVertexVertexMasks(Weight thisCoeff, Weight dstCoeff, USER_MASK& dst) const {

            //
            //  This implementation is convoluted by the potential sparsity of each mask.  Since
            //  it is specific to a vertex-vertex mask, we are guaranteed to have exactly one
            //  vertex-weight for both masks, but the edge- and face-weights are optional.  The
            //  child mask (the "source") should have a superset of the weights of the parent
            //  (the "destination") given its reduced sharpness, so we fortunately don't need to
            //  test all permutations.
            //
            dst.VertexWeight(0) = dstCoeff * dst.VertexWeight(0) + thisCoeff * this->VertexWeight(0);

            int edgeWeightCount = this->GetNumEdgeWeights();
            if (edgeWeightCount) {
                if (dst.GetNumEdgeWeights() == 0) {
                    dst.SetNumEdgeWeights(edgeWeightCount);
                    for (int i = 0; i < edgeWeightCount; ++i) {
                        dst.EdgeWeight(i) = thisCoeff * this->EdgeWeight(i);
                    }
                } else {
                    for (int i = 0; i < edgeWeightCount; ++i) {
                        dst.EdgeWeight(i) = dstCoeff * dst.EdgeWeight(i) + thisCoeff * this->EdgeWeight(i);
                    }
                }
            }

            int faceWeightCount = this->GetNumFaceWeights();
            if (faceWeightCount) {
                //
                //  If combining face weights, be sure their interpretation (i.e. face-centers
                //  or opposite vertices) is properly set in the destination mask:
                //
                if (dst.GetNumFaceWeights() == 0) {
                    dst.SetNumFaceWeights(faceWeightCount);
                    dst.SetFaceWeightsForFaceCenters(this->AreFaceWeightsForFaceCenters());

                    for (int i = 0; i < faceWeightCount; ++i) {
                        dst.FaceWeight(i) = thisCoeff * this->FaceWeight(i);
                    }
                } else {
                    assert(this->AreFaceWeightsForFaceCenters() == dst.AreFaceWeightsForFaceCenters());

                    for (int i = 0; i < faceWeightCount; ++i) {
                        dst.FaceWeight(i) = dstCoeff * dst.FaceWeight(i) + thisCoeff * this->FaceWeight(i);
                    }
                }
            }
        }

    private:
        Weight* _vWeights;
        Weight* _eWeights;
        Weight* _fWeights;
        int _vCount;
        int _eCount;
        int _fCount;
        bool _fWeightsForCenters;
    };
};


//
//  Crease and corner masks are common to most schemes -- the rest need to be provided
//  for each Scheme specialization.
//
template <Sdc::Type SCHEME>
template <typename EDGE, typename MASK>
inline void
Scheme<SCHEME>::assignCreaseMaskForEdge(EDGE const&, MASK& mask) const {

    mask.SetNumVertexWeights(2);
    mask.SetNumEdgeWeights(0);
    mask.SetNumFaceWeights(0);
    mask.SetFaceWeightsForFaceCenters(false);

    mask.VertexWeight(0) = 0.5f;
    mask.VertexWeight(1) = 0.5f;
}

template <Sdc::Type SCHEME>
template <typename VERTEX, typename MASK>
inline void
Scheme<SCHEME>::assignCornerMaskForVertex(VERTEX const&, MASK& mask) const {

    mask.SetNumVertexWeights(1);
    mask.SetNumEdgeWeights(0);
    mask.SetNumFaceWeights(0);
    mask.SetFaceWeightsForFaceCenters(false);

    mask.VertexWeight(0) = 1.0f;
}


//
//  The computation of a face-vertex mask is trivial and consistent for all schemes:
//
template <Sdc::Type SCHEME>
template <typename FACE, typename MASK>
void
Scheme<SCHEME>::ComputeFaceVertexMask(FACE const& face, MASK& mask) const {

    int vertCount = face.GetNumVertices();

    mask.SetNumVertexWeights(vertCount);
    mask.SetNumEdgeWeights(0);
    mask.SetNumFaceWeights(0);
    mask.SetFaceWeightsForFaceCenters(false);

    typename MASK::Weight vWeight = 1.0f / (typename MASK::Weight) vertCount;
    for (int i = 0; i < vertCount; ++i) {
        mask.VertexWeight(i) = vWeight;
    }
}


//
//  The computation of an edge-vertex mask requires inspection of sharpness values to
//  determine if smooth or a crease, and also to detect and apply a transition from a
//  crease to smooth.  Using the protected methods to assign the specific masks (only
//  two -- smooth or crease) this implementation should serve all non-linear schemes
//  (currently Catmark and Loop) and only need to be specialized it for Bilinear to
//  trivialize it to the crease case.
//
//  The implementation here is slightly complicated by combining two scenarios into a
//  single implementation -- either the caller knows the parent and child rules and
//  provides them, or they don't and the Rules have to be determined from sharpness
//  values.  Both cases include quick return once the parent is determined to be
//  smooth or the child a crease, leaving the transitional case remaining.
//
//  The overall process is as follows:
//
//      - quickly detect the most common specified or detected Smooth case and return
//      - quickly detect a full Crease by child Rule assignment and return
//      - determine from sharpness if unspecified child is a crease -- return if so
//      - compute smooth mask for child and combine with crease from parent
//
//  Usage of the parent Rule here allows some misuse in that only three of five possible
//  assignments are legitimate for the parent and four for the child (Dart being only
//  valid for the child and Corner for neither).  Results are undefined in these cases.
//
template <Sdc::Type SCHEME>
template <typename EDGE, typename MASK>
void
Scheme<SCHEME>::ComputeEdgeVertexMask(EDGE const&     edge,
                                         MASK&           mask,
                                         Crease::Rule parentRule,
                                         Crease::Rule childRule) const {

    //
    //  If the parent was specified or determined to be Smooth, we can quickly return
    //  with a Smooth mask.  Otherwise the parent is a crease -- if the child was
    //  also specified to be a crease, we can quickly return with a Crease mask.
    //
    if ((parentRule == Crease::RULE_SMOOTH) ||
       ((parentRule == Crease::RULE_UNKNOWN) && (edge.GetSharpness() <= 0.0f))) {
        assignSmoothMaskForEdge(edge, mask);
        return;
    }
    if (childRule == Crease::RULE_CREASE) {
        assignCreaseMaskForEdge(edge, mask);
        return;
    }

    //
    //  We have a Crease on the parent and the child was either specified as Smooth
    //  or was not specified at all -- deal with the unspecified case first (again
    //  returning a Crease mask if the child is also determined to be a Crease) and
    //  continue if we have a transition to Smooth.
    //
    //  Note when qualifying the child that if the parent sharpness > 1.0, regardless
    //  of the creasing method, whether the child sharpness values decay to zero is
    //  irrelevant -- the fractional weight for such a case (the value of the parent
    //  sharpness) is > 1.0, and when clamped to 1 effectively yields a full crease.
    //
    if (childRule == Crease::RULE_UNKNOWN) {
        Crease crease(_options);

        bool childIsCrease = false;
        if (parentRule == Crease::RULE_CREASE) {
            //  Child unknown as default value but parent Rule specified as Crease
            childIsCrease = true;
        } else if (edge.GetSharpness() >= 1.0f) {
            //  Sharpness >= 1.0 always a crease -- see note above
            childIsCrease = true;
        } else if (crease.IsUniform()) {
            //  Sharpness < 1.0 is guaranteed to decay to 0.0 for Uniform child edges
            childIsCrease = false;
        } else {
            //  Sharpness <= 1.0 does not necessarily decay to 0.0 for both child edges...
            float cEdgeSharpness[2];
            edge.GetChildSharpnesses(crease, cEdgeSharpness);
            childIsCrease = (cEdgeSharpness[0] > 0.0f) && (cEdgeSharpness[1] > 0.0f);
        }
        if (childIsCrease) {
            assignCreaseMaskForEdge(edge, mask);
            return;
        }
    }

    //
    //  We are now left with have the Crease-to-Smooth case -- compute the Smooth mask
    //  for the child and augment it with the transitional Crease of the parent.
    //
    //  A general combination of separately assigned masks here (as done in the vertex-
    //  vertex case) is overkill -- trivially combine the 0.5f vertex coefficient for
    //  the Crease of the parent with the vertex weights and attenuate the face weights
    //  accordingly.
    //
    assignSmoothMaskForEdge(edge, mask);

    typedef typename MASK::Weight Weight;

    Weight pWeight = edge.GetSharpness();
    Weight cWeight = 1.0f - pWeight;

    mask.VertexWeight(0) = pWeight * 0.5f + cWeight * mask.VertexWeight(0);
    mask.VertexWeight(1) = pWeight * 0.5f + cWeight * mask.VertexWeight(1);

    int faceCount = mask.GetNumFaceWeights();
    for (int i = 0; i < faceCount; ++i) {
        mask.FaceWeight(i) *= cWeight;
    }
}

//
//  The computation of a vertex-vertex mask requires inspection of creasing sharpness values
//  to determine what subdivision Rules apply to the parent and its child vertex, and also to
//  detect and apply a transition between two differing Rules.  Using the protected methods to
//  assign specific masks, this implementation should serve all non-linear schemes (currently
//  Catmark and Loop) and only need to be specialized for Bilinear to remove all unnecessary
//  complexity relating to creasing, Rules, etc.
//
//  The implementation here is slightly complicated by combining two scenarios into one --
//  either the caller knows the parent and child rules and provides them, or they don't and
//  the Rules have to be determined from sharpness values.  Even when the Rules are known and
//  provided though, there are cases where the parent and child sharpness values need to be
//  identified, so accounting for the unknown Rules too is not much of an added complication.
//
//  The benefit of supporting specified Rules is that they can often often be trivially
//  determined from context (e.g. a vertex derived from a face at a previous level will always
//  be smooth) rather than more generally, and at greater cost, inspecting neighboring and
//  they are often the same for parent and child.
//
//  The overall process is as follows:
//
//      - quickly detect the most common Smooth case when specified and return
//      - determine if sharpness for parent is required and gather if so
//      - if unspecified, determine the parent rule
//      - assign mask for the parent rule -- returning if Smooth/Dart
//      - return if child rule matches parent
//      - gather sharpness for child to determine or combine child rule
//      - if unspecified, determine the child rule, returning if it matches parent
//      - assign local mask for child rule
//      - combine local child mask with the parent mask
//
//  Remember -- if the parent rule is specified but the child is not, this implies only one
//  of the two optional rules was specified and is meant to indicate there is no transition,
//  so the child rule should be assigned to be the same (wish the compiler would allow this
//  in default value assignment).
//
template <Type SCHEME>
template <typename VERTEX, typename MASK>
void
Scheme<SCHEME>::ComputeVertexVertexMask(VERTEX const&   vertex,
                                           MASK&           mask,
                                           Crease::Rule pRule,
                                           Crease::Rule cRule) const {

    //  Quick assignment and return for the most common case:
    if ((pRule == Crease::RULE_SMOOTH) || (pRule == Crease::RULE_DART)) {
        assignSmoothMaskForVertex(vertex, mask);
        return;
    }
    //  If unspecified, assign the child rule to match the parent rule if specified:
    if ((cRule == Crease::RULE_UNKNOWN) && (pRule != Crease::RULE_UNKNOWN)) {
        cRule = pRule;
    }
    int valence = vertex.GetNumEdges();

    //
    //  Determine if we need the parent edge sharpness values -- identify/gather if so
    //  and use it to compute the parent rule if unspecified:
    //
    float * pEdgeSharpnessBuffer = (float *)alloca(valence*sizeof(float)),
          * pEdgeSharpness   = 0,
            pVertexSharpness = 0.0f;

    bool requireParentSharpness = (pRule == Crease::RULE_UNKNOWN) ||
                                  (pRule == Crease::RULE_CREASE) ||
                                  (pRule != cRule);
    if (requireParentSharpness) {
        pVertexSharpness = vertex.GetSharpness();
        pEdgeSharpness   = vertex.GetSharpnessPerEdge(pEdgeSharpnessBuffer);

        if (pRule == Crease::RULE_UNKNOWN) {
            Crease crease(_options);
            pRule = crease.DetermineVertexVertexRule(pVertexSharpness, valence, pEdgeSharpness);
        }
    }
    if ((pRule == Crease::RULE_SMOOTH) || (pRule == Crease::RULE_DART)) {
        assignSmoothMaskForVertex(vertex, mask);
        return;  //  As done on entry, we can return immediately if parent is Smooth/Dart
    } else if (pRule == Crease::RULE_CREASE) {
        assignCreaseMaskForVertex(vertex, mask, pEdgeSharpness);
    } else {
        assignCornerMaskForVertex(vertex, mask);
    }
    if (cRule == pRule) return;

    //
    //  Identify/gather child sharpness to combine masks for the two differing Rules:
    //
    Crease crease(_options);

    float * cEdgeSharpnessBuffer = (float *)alloca(valence*sizeof(float)),
          * cEdgeSharpness = vertex.GetChildSharpnessPerEdge(crease, cEdgeSharpnessBuffer),
            cVertexSharpness = vertex.GetChildSharpness(crease);

    if (cRule == Crease::RULE_UNKNOWN) {
        cRule = crease.DetermineVertexVertexRule(cVertexSharpness, valence, cEdgeSharpness);
        if (cRule == pRule) return;
    }

    //
    //  Intialize a local child mask, compute the fractional weight from parent and child
    //  sharpness values and combine the two masks:
    //
    typedef typename MASK::Weight Weight;

    Weight * cMaskWeights = (Weight *)alloca((1 + 2 * valence)*sizeof(Weight));
    LocalMask<Weight> cMask(cMaskWeights, cMaskWeights + 1, cMaskWeights + 1 + valence);

    if ((cRule == Crease::RULE_SMOOTH) || (cRule == Crease::RULE_DART)) {
        assignSmoothMaskForVertex(vertex, cMask);
    } else if (cRule == Crease::RULE_CREASE) {
        assignCreaseMaskForVertex(vertex, cMask, cEdgeSharpness);
    } else {
        assignCornerMaskForVertex(vertex, cMask);
    }

    Weight pWeight = crease.ComputeFractionalWeightAtVertex(pVertexSharpness, cVertexSharpness,
                                                   valence, pEdgeSharpness, cEdgeSharpness);
    Weight cWeight = 1.0f - pWeight;

    cMask.CombineVertexVertexMasks(cWeight, pWeight, mask);
}

//
//  The computation of limit masks for vertices:
//
template <Type SCHEME>
template <typename VERTEX, typename MASK>
void
Scheme<SCHEME>::ComputeVertexLimitMask(VERTEX const& vertex,
                                       MASK&         mask) const
{
    if (vertex.GetNumFaces() == vertex.GetNumEdges()) {
        assignInteriorLimitMask(vertex, mask);
    } else if ((vertex.GetNumFaces() == 1) &&
               (_options.GetVVarBoundaryInterpolation() == Sdc::Options::VVAR_BOUNDARY_EDGE_AND_CORNER)) {
        assignCornerMaskForVertex(vertex, mask);
    } else {
        assignBoundaryLimitMask(vertex, mask);
    }
}

template <Type SCHEME>
template <typename VERTEX, typename MASK>
void
Scheme<SCHEME>::ComputeVertexLimitMask(VERTEX const& vertex,
                                       MASK&         posMask,
                                       MASK&         tan1Mask,
                                       MASK&         tan2Mask) const
{
    if (vertex.GetNumFaces() == vertex.GetNumEdges()) {
        assignInteriorLimitMask(vertex, posMask);
        assignInteriorLimitTangentMasks(vertex, tan1Mask, tan2Mask);
    } else {
        assignBoundaryLimitMask(vertex, posMask);
        assignBoundaryLimitTangentMasks(vertex, tan1Mask, tan2Mask);
    }
}


} // end namespace sdc

} // end namespace OPENSUBDIV_VERSION
using namespace OPENSUBDIV_VERSION;
} // end namespace OpenSubdiv

#endif /* SDC_SCHEME_H */
