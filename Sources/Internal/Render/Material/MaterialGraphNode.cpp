/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Material/MaterialGraphNode.h"

#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"
#include "Render/Shader.h"

namespace DAVA
{

/*
    Code for some of the nodes:

    Mul:
    var [name] = [a] * [b];
    
    Add:
    var [name] = [a] + [b];
    
    Lerp:
    var [name] = lerp([a], [b], [t]);
 
    <ambient diffuse>
    </>
    
    <diffuse> <diffuse>
 
 */
    
MaterialGraphNode::MaterialGraphNode()
    :   type(TYPE_NONE)
    ,   depthMarker(0)
    ,   isVertexShaderNode(false)
    ,   usedByOthersModifier()
    ,   textureInputIndex(0)
    ,   textureChannelIndex(0)
    ,   shader(0)
{
    
}
    
MaterialGraphNode::~MaterialGraphNode()
{
    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
    {
        SafeRelease(it->second);
    }
    inputConnectors.clear();
}
    
void MaterialGraphNode::InitFromYamlNode(YamlNode * graphNode)
{
    YamlNode * typeNode = graphNode->Get("node");
    YamlNode * nameNode = graphNode->Get("name");
    SetType(typeNode->AsString());
    SetName(nameNode->AsString());
        
    if (type == TYPE_SAMPLE_2D)
    {
        YamlNode * inputNode = graphNode->Get("input");
        if (inputNode)
            textureInputIndex = inputNode->AsInt();
        
        YamlNode * textureChannelNode = graphNode->Get("channel");
        if (textureChannelNode)
            textureChannelIndex = textureChannelNode->AsInt();
    }
}

void MaterialGraphNode::SetDepthMarker(uint32 _depthMarker)
{
    depthMarker = _depthMarker;
}
    
uint32 MaterialGraphNode::GetDepthMarker()
{
    return depthMarker;
}

Map<String, MaterialGraphNodeConnector*> & MaterialGraphNode::GetInputConnectors()
{
    return inputConnectors;
}

    
void MaterialGraphNode::ConnectToNode(const String & connectorName, MaterialGraphNode * node, const String & connectionModifier)
{
    MaterialGraphNodeConnector * connector = new MaterialGraphNodeConnector();
    connector->node = node;
    connector->modifier = connectionModifier;
    inputConnectors[connectorName] = connector;
    node->MergeConnectionModifiers(connectionModifier);
}
    
void MaterialGraphNode::MergeConnectionModifiers(const String & usedByOtherNode)
{
    usedByOthersModifier += usedByOtherNode;
    //std::sort(usedByOthersModifier.begin(), usedByOthersModifier.end());
    std::unique(usedByOthersModifier.begin(), usedByOthersModifier.end());
}

MaterialGraphNodeConnector * MaterialGraphNode::GetInputConnector(const String & name)
{
    Map<String, MaterialGraphNodeConnector*>::iterator res = inputConnectors.find(name);
    if (res != inputConnectors.end())
        return res->second;
    return 0;
}

static const char * returnTypes[] =
{
    "wrong type",
    "float",
    "vec2",
    "vec3",
    "vec4",
};

String MaterialGraphNode::GetResultFormat(const String & s1, const String & s2)
{
    uint32 s1Len = s1.length();
    uint32 s2Len = s2.length();
    if ((s1Len == 1) || (s2Len == 1))
    {
        uint32 formatBytes = Max(s1Len, s2Len);
        return returnTypes[formatBytes];
    }
    else if (s1Len == s2Len)return returnTypes[s1Len];
    return returnTypes[0];
}

MaterialGraphNode::eCompileError MaterialGraphNode::GenerateCode(String & vertexShader, String & pixelShader)
{
    Logger::Debug("Generate Code: %d", type);
    
    
    uint32 usedByOthersCount = usedByOthersModifier.length();
    /*     if (usedByOthersCount == 0)
         {
            return ERROR_UNUSED_NODE;
         }
     */
    //
    if (type == TYPE_SAMPLE_2D)
    {
        
        nodeCode = Format("%s %s = texture2D(texture[%d], varTexCoord[%d]).%s;", returnTypes[usedByOthersCount],
                                    name.c_str(),
                                    textureChannelIndex,
                                    textureInputIndex,
                                    usedByOthersModifier.c_str());
        Logger::Debug("%s", nodeCode.c_str());
        if (isVertexShaderNode)
            vertexShader += nodeCode;
        else
            pixelShader += nodeCode;
    }
    if (type == TYPE_MUL)
    {
        MaterialGraphNodeConnector * connectorA = GetInputConnector("a");
        MaterialGraphNodeConnector * connectorB = GetInputConnector("b");
        
        if (!connectorA || !connectorB)
            return ERROR_NOT_ENOUGH_CONNECTORS;
        String resultFormat = GetResultFormat(connectorA->modifier, connectorB->modifier);
        
        String op1 = connectorA->node->GetName();
        if (!connectorA->modifier.empty())op1 += "." + connectorA->modifier;
        String op2 = connectorB->node->GetName();
        if (!connectorB->modifier.empty())op2 += "." + connectorB->modifier;
        
        nodeCode = Format("%s %s = %s * %s;",  resultFormat.c_str(),
                                                    name.c_str(),
                                                    op1.c_str(),
                                                    op2.c_str());
        Logger::Debug("%s", nodeCode.c_str());
        if (isVertexShaderNode)
            vertexShader += nodeCode;
        else
            pixelShader += nodeCode;
    
    }
    if (type == TYPE_ADD)
    {
        MaterialGraphNodeConnector * connectorA = GetInputConnector("a");
        MaterialGraphNodeConnector * connectorB = GetInputConnector("b");
        
        if (!connectorA || !connectorB)
            return ERROR_NOT_ENOUGH_CONNECTORS;
        String resultFormat = GetResultFormat(connectorA->modifier, connectorB->modifier);
        
        nodeCode = Format("%s %s = %s.%s + %s.%s;", resultFormat.c_str(),
                            name.c_str(),
                            connectorA->node->GetName().c_str(),
                            connectorA->modifier.c_str(),
                            connectorB->node->GetName().c_str(),
                            connectorB->modifier.c_str());

        Logger::Debug("%s", nodeCode.c_str());
        if (isVertexShaderNode)
            vertexShader += nodeCode;
        else
            pixelShader += nodeCode;
    }
    
    if (type == TYPE_FORWARD_MATERIAL)
    {
        // shader = new Shader();
        MaterialGraphNodeConnector * connectorEmissive = GetInputConnector("emissive");
        MaterialGraphNodeConnector * connectorDiffuse = GetInputConnector("diffuse");
        MaterialGraphNodeConnector * connectorSpecular = GetInputConnector("specular");
        MaterialGraphNodeConnector * connectorNormal = GetInputConnector("normal");
//      MaterialGraphNodeConnector * connectorAlpha = GetInputConnector("alpha");
        
        // vertexShader +=
        fragmentShaderCode += "#define GRAPH_CUSTOM_PIXEL_CODE ";
        fragmentShaderCode += pixelShader;
        fragmentShaderCode += "\n";
        
        if (connectorEmissive)
            fragmentShaderCode += Format("#define IN_EMISSIVE %s\n", connectorEmissive->GetNode()->GetName().c_str());
        if (connectorDiffuse)
            fragmentShaderCode += Format("#define IN_DIFFUSE %s\n", connectorDiffuse->GetNode()->GetName().c_str());
        if (connectorSpecular)
            fragmentShaderCode += Format("#define IN_SPECULAR %s\n", connectorSpecular->GetNode()->GetName().c_str());
        if (connectorNormal)
            fragmentShaderCode += Format("#define IN_NORMAL %s\n", connectorNormal->GetNode()->GetName().c_str());
        
        if (connectorNormal)
        {
            fragmentShaderCode += "#define PIXEL_LIT\n";
            vertexShaderCode += "#define PIXEL_LIT\n";
        }
        else if (connectorDiffuse || connectorSpecular)
        {
            fragmentShaderCode += "#define VERTEX_LIT\n";
            vertexShaderCode += "#define VERTEX_LIT\n";
        }
        
        
    }
    
    return NO_ERROR;
}

const String & MaterialGraphNode::GetName() const
{
    return name;
}
    
void MaterialGraphNode::SetName(const String & _name)
{
    name = _name;
}

static const char * types[] =
{
    "TYPE_NONE",
    "TYPE_FORWARD_MATERIAL",
    "TYPE_DEFERRED_MATERIAL",
    "TYPE_SAMPLE_2D",
    "TYPE_MUL",
    "TYPE_ADD",
    "TYPE_LERP",
    "TYPE_TIME",
    "TYPE_SIN",
    "TYPE_COS",
};

void MaterialGraphNode::SetType(const String & _type)
{
    for (uint32 k = 0; k < TYPE_COUNT; ++k)
    {
        if (_type == types[k])
        {
            type = (eType)k;
            return;
        }
    }
    DVASSERT(0 && "MaterialGraphNode type not found");
}


};

