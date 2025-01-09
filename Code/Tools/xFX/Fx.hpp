// Author: Jake Rieger
// Created: 1/9/2025.
//

#pragma once

#include "Types.hpp"
#include "Panic.inl"
#include "Filesystem.hpp"

#include <variant>
#include <shaderc/shaderc.hpp>
#include <nlohmann/json.hpp>

namespace x {
    using Bytecode = std::vector<u8>;

    struct SpecializationConstant {
        u32 id;
        str name;
        str type;  // "bool", "int", "float", etc.
        std::variant<bool, int, float> defaultValue;
    };

    struct ShaderStage {
        str entryPoint;
        Bytecode bytecode;
        std::vector<SpecializationConstant> specializationConstants;
    };

    struct PipelineConfig {};

    class Effect {
    public:
        str name;
        str description;
        std::vector<str> tags;
        std::optional<ShaderStage> vertexStage;
        std::optional<ShaderStage> fragmentStage;
        std::optional<ShaderStage> computeStage;
        PipelineConfig pipelineConfig;

        bool IsCompute() const;
        bool IsGraphics() const;
    };

    class Fx {
    public:
        static Effect LoadFromXFX(const Filesystem::Path& xfxFile);

    private:
        static shaderc::Compiler& GetCompiler();
        static shaderc_shader_kind GetShaderKind(const str& extension);
        static Bytecode CompileShader(const Filesystem::Path& sourcePath, const str& entryPoint);
    };
}  // namespace x
