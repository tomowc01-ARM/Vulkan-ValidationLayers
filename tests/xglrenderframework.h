/*
 * XGL Tests
 *
 * Copyright (C) 2014 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   Courtney Goeltzenleuchter <courtney@lunarg.com>
 */

#ifndef XGLRENDERFRAMEWORK_H
#define XGLRENDERFRAMEWORK_H

#include "xgltestframework.h"

class XglDevice : public xgl_testing::Device
{
public:
    XglDevice(XGL_UINT id, XGL_PHYSICAL_GPU obj);

    XGL_DEVICE device() { return obj(); }
    void get_device_queue();

    XGL_UINT id;
    XGL_PHYSICAL_GPU_PROPERTIES props;
    const XGL_PHYSICAL_GPU_QUEUE_PROPERTIES *queue_props;

    XGL_QUEUE m_queue;
};


class XglRenderFramework : public XglTestFramework
{
public:
    XglRenderFramework();
    ~XglRenderFramework();

    XGL_DEVICE device() {return m_device->device();}
    XGL_PHYSICAL_GPU gpu() {return objs[0];}
    void InitViewport(float width, float height);
    void InitViewport();
    void InitRenderTarget();
    void InitFramework();
    void ShutdownFramework();
    void InitState();
    void GenerateClearAndPrepareBufferCmds();
    void GenerateBindRenderTargetCmd();
    void GenerateBindStateAndPipelineCmds();


protected:
    XGL_APPLICATION_INFO            app_info;
    XGL_PHYSICAL_GPU                objs[XGL_MAX_PHYSICAL_GPUS];
    XGL_UINT                        gpu_count;
    XglDevice                      *m_device;
    XGL_CMD_BUFFER                  m_cmdBuffer;
    XGL_MEMORY_REF                  m_memRefs[5];
    XGL_RASTER_STATE_OBJECT         m_stateRaster;
    XGL_COLOR_BLEND_STATE_OBJECT    m_colorBlend;
    XGL_VIEWPORT_STATE_OBJECT       m_stateViewport;
    XGL_DEPTH_STENCIL_STATE_OBJECT  m_stateDepthStencil;
    XGL_MSAA_STATE_OBJECT           m_stateMsaa;
    XglImage                       *m_renderTargets[XGL_MAX_COLOR_ATTACHMENTS];
    XGL_UINT                        m_renderTargetCount;
    XGL_FLOAT                       m_width, m_height;
    XGL_FORMAT                      m_render_target_fmt;
    XGL_COLOR_ATTACHMENT_BIND_INFO  m_colorBindings[XGL_MAX_COLOR_ATTACHMENTS];
    XGL_DEPTH_STENCIL_BIND_INFO     m_depthStencilBinding;

    /*
     * SetUp and TearDown are called by the Google Test framework
     * to initialize a test framework based on this class.
     */
    virtual void SetUp() {
        this->app_info.sType = XGL_STRUCTURE_TYPE_APPLICATION_INFO;
        this->app_info.pNext = NULL;
        this->app_info.pAppName = "base";
        this->app_info.appVersion = 1;
        this->app_info.pEngineName = "unittest";
        this->app_info.engineVersion = 1;
        this->app_info.apiVersion = XGL_MAKE_VERSION(0, 22, 0);

        InitFramework();
    }

    virtual void TearDown() {
        ShutdownFramework();
    }
};

class XglIndexBufferObj;
class XglConstantBufferObj;

class XglCommandBufferObj : xgl_testing::CmdBuffer
{
public:
    XglCommandBufferObj(XglDevice *device);
    XGL_CMD_BUFFER GetBufferHandle();
    XGL_RESULT BeginCommandBuffer(XGL_FLAGS flags);
    XGL_RESULT EndCommandBuffer();
    void PrepareMemoryRegions(int transitionCount, XGL_MEMORY_STATE_TRANSITION *transitionPtr);
    void AddRenderTarget(XglImage *renderTarget);
    void AddDepthStencil();
    void ClearAllBuffers();
    void ClearAllBuffers(XGL_DEPTH_STENCIL_BIND_INFO *depthStencilBinding, XGL_IMAGE depthStencilImage);
    void BindAttachments(XGL_DEPTH_STENCIL_BIND_INFO *depthStencilBinding);
    void BindAttachments();
    void BindPipeline(XGL_PIPELINE pipeline);
    void BindDescriptorSet(XGL_DESCRIPTOR_SET descriptorSet);
    void BindVertexBuffer(XglConstantBufferObj *vertexBuffer, XGL_UINT offset, XGL_UINT binding);
    void BindIndexBuffer(XglIndexBufferObj *indexBuffer, XGL_UINT offset);
    void BindState(XGL_RASTER_STATE_OBJECT stateRaster, XGL_VIEWPORT_STATE_OBJECT stateViewport,
                   XGL_COLOR_BLEND_STATE_OBJECT colorBlend, XGL_DEPTH_STENCIL_STATE_OBJECT stateDepthStencil,
                   XGL_MSAA_STATE_OBJECT stateMsaa);
    void Draw(XGL_UINT firstVertex, XGL_UINT vertexCount, XGL_UINT firstInstance, XGL_UINT instanceCount);
    void DrawIndexed(XGL_UINT firstIndex, XGL_UINT indexCount, XGL_INT vertexOffset, XGL_UINT firstInstance, XGL_UINT instanceCount);
    void QueueCommandBuffer(XGL_MEMORY_REF *memRefs, XGL_UINT32 numMemRefs);

protected:
    XglDevice                      *m_device;
    vector<XglImage*>               m_renderTargets;
    XGL_UINT                        m_renderTargetCount;

};

class XglConstantBufferObj : public xgl_testing::GpuMemory
{
public:
    XglConstantBufferObj(XglDevice *device);
    XglConstantBufferObj(XglDevice *device, int constantCount, int constantSize, const void* data);
    void SetMemoryState(XGL_MEMORY_STATE newState);
    void Bind(XGL_CMD_BUFFER cmdBuffer, XGL_GPU_SIZE offset, XGL_UINT binding);
    XGL_MEMORY_VIEW_ATTACH_INFO     m_constantBufferView;

protected:
    XglDevice                      *m_device;
    int                             m_numVertices;
    int                             m_stride;
    XglCommandBufferObj             *m_commandBuffer;
    xgl_testing::Fence              m_fence;
};

class XglIndexBufferObj : public XglConstantBufferObj
{
public:
    XglIndexBufferObj(XglDevice *device);
    void CreateAndInitBuffer(int numIndexes, XGL_INDEX_TYPE dataFormat, const void* data);
    void Bind(XGL_CMD_BUFFER cmdBuffer, XGL_GPU_SIZE offset);
    XGL_INDEX_TYPE GetIndexType();

protected:
    XGL_INDEX_TYPE  m_indexType;
};

class XglImage : public xgl_testing::Image
{
public:
    XglImage(XglDevice *dev);

    // Image usage flags
    //    typedef enum _XGL_IMAGE_USAGE_FLAGS
    //    {
    //        XGL_IMAGE_USAGE_SHADER_ACCESS_READ_BIT                  = 0x00000001,
    //        XGL_IMAGE_USAGE_SHADER_ACCESS_WRITE_BIT                 = 0x00000002,
    //        XGL_IMAGE_USAGE_COLOR_ATTACHMENT_BIT                    = 0x00000004,
    //        XGL_IMAGE_USAGE_DEPTH_STENCIL_BIT                       = 0x00000008,
    //    } XGL_IMAGE_USAGE_FLAGS;
public:
    void init( XGL_UINT32 w, XGL_UINT32 h,
                     XGL_FORMAT fmt, XGL_FLAGS usage,
                     XGL_IMAGE_TILING tiling=XGL_LINEAR_TILING);

    //    void clear( CommandBuffer*, XGL_UINT[4] );
    //    void prepare( CommandBuffer*, XGL_IMAGE_STATE );

    void state( XGL_IMAGE_STATE state )
    {
        m_imageInfo.state = state;
    }
    XGL_GPU_MEMORY memory() const
    {
        const std::vector<XGL_GPU_MEMORY> mems = memories();
        return mems.empty() ? XGL_NULL_HANDLE : mems[0];
    }


    XGL_IMAGE image() const
    {
        return obj();
    }
    XGL_COLOR_ATTACHMENT_VIEW targetView()const
    {
        return m_targetView.obj();
    }

    XGL_IMAGE_STATE state() const
    {
        return ( XGL_IMAGE_STATE )m_imageInfo.state;
    }
    XGL_UINT32 width() const
    {
        return extent().width;
    }
    XGL_UINT32 height() const
    {
        return extent().height;
    }

    XGL_RESULT MapMemory(XGL_VOID** ptr);
    XGL_RESULT UnmapMemory();

protected:
    XglDevice *m_device;

    xgl_testing::ColorAttachmentView m_targetView;
    XGL_IMAGE_VIEW_ATTACH_INFO   m_imageInfo;
};

class XglTextureObj : public xgl_testing::Image
{
public:
    XglTextureObj(XglDevice *device);
    void ChangeColors(uint32_t color1, uint32_t color2);
    XGL_IMAGE_VIEW_ATTACH_INFO m_textureViewInfo;

protected:
    XglDevice                 *m_device;
    xgl_testing::ImageView     m_textureView;
    XGL_GPU_SIZE               m_rowPitch;
};

class XglSamplerObj : public xgl_testing::Sampler
{
public:
    XglSamplerObj(XglDevice *device);

protected:
     XglDevice *m_device;

};

class XglDescriptorSetObj : public xgl_testing::DescriptorSet
{
public:
    XglDescriptorSetObj(XglDevice *device);
    void AttachMemoryView(XglConstantBufferObj* constantBuffer);
    void AttachSampler( XglSamplerObj* sampler);
    void AttachImageView( XglTextureObj* texture);
    void BindCommandBuffer(XGL_CMD_BUFFER commandBuffer);
    void CreateXGLDescriptorSet();
    XGL_DESCRIPTOR_SET GetDescriptorSetHandle();
    int GetTotalSlots();
    XGL_DESCRIPTOR_SLOT_INFO * GetSlotInfo(vector<int>slots, vector<XGL_DESCRIPTOR_SET_SLOT_TYPE>types, vector<void*>objs );

protected:
    XglDevice                           *m_device;
    XGL_DESCRIPTOR_SLOT_INFO            *m_slotInfo;
    int                                  m_nextSlot;
    vector<int>                          m_memorySlots;
    vector<XGL_MEMORY_VIEW_ATTACH_INFO*> m_memoryViews;
    vector<int>                          m_samplerSlots;
    vector<XglSamplerObj*>               m_samplers;
    vector<int>                          m_imageSlots;
    vector<XGL_IMAGE_VIEW_ATTACH_INFO*>  m_imageViews;
};


class XglShaderObj : public xgl_testing::Shader
{
public:
    XglShaderObj(XglDevice *device, const char * shaderText, XGL_PIPELINE_SHADER_STAGE stage, XglRenderFramework *framework);
    XGL_PIPELINE_SHADER_STAGE_CREATE_INFO* GetStageCreateInfo(XglDescriptorSetObj *descriptorSet);
    void BindShaderEntitySlotToMemory(int slot, XGL_DESCRIPTOR_SET_SLOT_TYPE type, XglConstantBufferObj *constantBuffer);
    void BindShaderEntitySlotToImage(int slot, XGL_DESCRIPTOR_SET_SLOT_TYPE type, XglTextureObj *texture);
    void BindShaderEntitySlotToSampler(int slot, XglSamplerObj *sampler);

protected:
    XGL_PIPELINE_SHADER_STAGE_CREATE_INFO stage_info;
    XGL_PIPELINE_SHADER_STAGE m_stage;
    XglDevice *m_device;
    vector<int>    m_memSlots;
    vector<XGL_DESCRIPTOR_SET_SLOT_TYPE> m_memTypes;
    vector<XGL_MEMORY_VIEW_ATTACH_INFO*> m_memObjs;
    vector<int>    m_samplerSlots;
    vector<XGL_DESCRIPTOR_SET_SLOT_TYPE> m_samplerTypes;
    vector<XglSamplerObj*> m_samplerObjs;
    vector<int>    m_imageSlots;
    vector<XGL_DESCRIPTOR_SET_SLOT_TYPE> m_imageTypes;
    vector<XGL_IMAGE_VIEW_ATTACH_INFO*> m_imageObjs;

};

class XglPipelineObj : public xgl_testing::Pipeline
{
public:
    XglPipelineObj(XglDevice *device);
    void BindPipelineCommandBuffer(XGL_CMD_BUFFER m_cmdBuffer, XglDescriptorSetObj *descriptorSet);
    void AddShader(XglShaderObj* shaderObj);
    void AddVertexInputAttribs(XGL_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION* vi_attrib, int count);
    void AddVertexInputBindings(XGL_VERTEX_INPUT_BINDING_DESCRIPTION* vi_binding, int count);
    void AddVertexDataBuffer(XglConstantBufferObj* vertexDataBuffer, int binding);
    void SetColorAttachment(XGL_UINT binding, const XGL_PIPELINE_CB_ATTACHMENT_STATE *att);
    void CreateXGLPipeline(XglDescriptorSetObj *descriptorSet);
    XGL_PIPELINE GetPipelineHandle();

protected:
    XGL_PIPELINE_VERTEX_INPUT_CREATE_INFO m_vi_state;
    XGL_PIPELINE_IA_STATE_CREATE_INFO m_ia_state;
    XGL_PIPELINE_RS_STATE_CREATE_INFO m_rs_state;
    XGL_PIPELINE_CB_STATE m_cb_state;
    XGL_PIPELINE_DB_STATE_CREATE_INFO m_db_state;
    XglDevice *m_device;
    vector<XglShaderObj*> m_shaderObjs;
    vector<XglConstantBufferObj*> m_vertexBufferObjs;
    vector<int> m_vertexBufferBindings;
    int m_vertexBufferCount;

};
class XglMemoryRefManager{
public:
    XglMemoryRefManager();
    void AddMemoryRef(XglConstantBufferObj* constantBuffer);
    void AddMemoryRef(XglTextureObj *texture);
    XGL_MEMORY_REF* GetMemoryRefList();
    int GetNumRefs();

protected:
    int m_numRefs;
    vector<XGL_GPU_MEMORY> m_bufferObjs;

};


#endif // XGLRENDERFRAMEWORK_H
