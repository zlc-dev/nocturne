#pragma once

#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <assimp/Importer.hpp>
#include <cstddef>
#include <tuple>
#include <vector>

class Model {
public:
    void loadModelFromMemory(const void *p_buffer, size_t length) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(
            p_buffer, length, 
            aiProcess_Triangulate | aiProcess_FlipUVs
        );
        if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            fprintf(stderr, "Error loading model: %s\n", importer.GetErrorString());
            return;
        }

        const aiMesh* mesh = scene->mMeshes[0];
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            m_vertices.push_back({
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            });
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                m_indices.push_back(face.mIndices[j]);
            }
        }
    }
    
public:
    std::vector<std::tuple<float, float, float>> m_vertices {};
    std::vector<unsigned int> m_indices;
};
