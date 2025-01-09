// Author: Jake Rieger
// Created: 1/9/2025.
//

#include "Fx.hpp"

namespace x {
    bool Effect::IsCompute() const {
        return computeStage.has_value();
    }

    bool Effect::IsGraphics() const {
        return vertexStage.has_value() && fragmentStage.has_value();
    }

    Effect Fx::LoadFromXFX(const Filesystem::Path& xfxFile) {
        using json = nlohmann::json;
        using Filesystem::Path;

        str xfxSource = Filesystem::FileReader::ReadAllText(xfxFile.Str());
        json schema   = json::parse(xfxSource);

        Effect effect;
        effect.name        = schema["name"].get<str>();
        effect.description = schema["description"].get<str>();
        if (schema.contains("tags")) { effect.tags = schema["tags"].get<std::vector<str>>(); }

        if (const str type = schema["type"].get<str>(); type == "compute") {
            // TODO: Implement parsing compute shader schemas
        } else if (type == "graphics") {
            const auto& graphics = schema["graphics"];
            if (graphics.contains("vertex")) {
                ShaderStage stage;
                stage.entryPoint      = graphics["vertex"]["entry"].get<str>();
                const auto sourcePath = xfxFile.Parent() / graphics["vertex"]["source"].get<str>();
                stage.bytecode        = CompileShader(sourcePath, stage.entryPoint);
                // TODO: Parse specialization constants
                effect.vertexStage = std::move(stage);
            }

            if (graphics.contains("fragment")) {
                ShaderStage stage;
                stage.entryPoint = graphics["fragment"]["entry"].get<str>();
                const auto sourcePath =
                  xfxFile.Parent() / graphics["fragment"]["source"].get<str>();
                stage.bytecode = CompileShader(sourcePath, stage.entryPoint);
                // TODO: Parse specialization constants
                effect.fragmentStage = std::move(stage);
            }
        }

        // TODO: Parse pipeline configuration

        return effect;
    }

    shaderc::Compiler& Fx::GetCompiler() {
        static shaderc::Compiler compiler;
        return compiler;
    }

    shaderc_shader_kind Fx::GetShaderKind(const str& extension) {
        if (extension == "vert") return shaderc_vertex_shader;
        if (extension == "frag") return shaderc_fragment_shader;
        if (extension == "comp") return shaderc_compute_shader;
        Panic("Unknown shader type for file extension: %s", extension);
    }

    Bytecode Fx::CompileShader(const Filesystem::Path& sourcePath, const str& entryPoint) {
        const str sourceCode = Filesystem::FileReader::ReadAllText(sourcePath.Str());
        const auto extension = sourcePath.Extension();
        shaderc::CompileOptions options;
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
        const auto result = GetCompiler().CompileGlslToSpv(sourceCode,
                                                           GetShaderKind(extension),
                                                           sourcePath.CStr(),
                                                           options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            Panic(result.GetErrorMessage().c_str());
        }

        Bytecode bytecode(RCAST<const u8*>(result.cbegin()), RCAST<const u8*>(result.cend()));
        return bytecode;
    }
}  // namespace x