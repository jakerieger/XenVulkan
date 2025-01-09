import json
import os
import subprocess
import glob


def load_config(config_path: str):
    try:
        with open(config_path, 'r') as f:
            config = json.load(f)
        required_fields = ['glslc_path', 'source', 'output']
        for field in required_fields:
            if field not in config:
                raise ValueError(f"Missing required field: {field}")
        if not os.path.exists(config['glslc_path']):
            raise FileNotFoundError(f"glslc compiler not found at: {config['glslc_path']}")
        config['source'] = os.path.expandvars(config['source'])
        config['output'] = os.path.expandvars(config['output'])
        return config
    except json.JSONDecodeError:
        raise ValueError("Invalid configuration file")


def compile_shader(glslc_path: str, shader_path: str, output_path: str):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    try:
        result = subprocess.run([glslc_path, shader_path, '-o', output_path], capture_output=True,
                                text=True)
        if result.returncode != 0:
            print(f"Error compiling {shader_path}:")
            print(result.stderr)
            return False
        return True
    except subprocess.SubprocessError as e:
        print(f"Failed to execute glslc for {shader_path}: {e}")
        return False


def main():
    try:
        config = load_config("compile_shaders.json")
    except Exception as e:
        print(f"Error loading configuration: {e}")
        return
    shader_extensions = ['.vert', '.frag', '.comp', '.geom', '.tesc', '.tese']
    shader_files = []
    for ext in shader_extensions:
        shader_files.extend(
            glob.glob(os.path.join(config['source'], f"**/*{ext}"), recursive=True))
    if not shader_files:
        print(f"No shader sources were found in {config['source']}")
        return
    success_count = 0
    total_count = len(shader_files)
    for shader_path in shader_files:
        rel_path = os.path.relpath(shader_path, config['source'])
        output_path = os.path.join(config['output'], rel_path + '.spv')
        print(f"Compiling: {rel_path}")
        if compile_shader(config['glslc_path'], shader_path, output_path):
            success_count += 1
    print(f"\nCompilation complete: {success_count}/{total_count} shaders compiled successfully.")


if __name__ == '__main__':
    main()
