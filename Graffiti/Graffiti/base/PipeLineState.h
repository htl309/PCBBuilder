#pragma once

namespace Graffiti {



    enum class PipelineState : uint32_t
    {
        Common = 0,
        WireFrame = GF_BIT(0),	//0000 0001
        DepthTestDisable = GF_BIT(1),			//0000 0010
        MeshShaderPipeLine= GF_BIT(2),
        // �ɼ�����չ����״̬...
    };
    // ��λ��
    inline PipelineState operator|(PipelineState a, PipelineState b)
    {
        return static_cast<PipelineState>(
            static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    // ��λ��
    inline PipelineState operator&(PipelineState a, PipelineState b)
    {
        return static_cast<PipelineState>(
            static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    // ��λȡ��
    inline PipelineState operator~(PipelineState a)
    {
        return static_cast<PipelineState>(
            ~static_cast<uint32_t>(a));
    }

    // ��򣨿�ѡ��
    inline PipelineState operator^(PipelineState a, PipelineState b)
    {
        return static_cast<PipelineState>(
            static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
    }

    // ����������� |=
    inline PipelineState& operator|=(PipelineState& a, PipelineState b)
    {
        a = a | b;
        return a;
    }

    // ����������� &=
    inline PipelineState& operator&=(PipelineState& a, PipelineState b)
    {
        a = a & b;
        return a;
    }

    // ����������� ^=
    inline PipelineState& operator^=(PipelineState& a, PipelineState b)
    {
        a = a ^ b;
        return a;
    }

    inline bool HasState(PipelineState value, PipelineState test)
    {
        return (value & test) != PipelineState::Common;
    }

}