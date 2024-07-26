#include "Compiler.hpp"
#include <regex>
#include "Console.hpp"
#include "FileHandler.hpp"
#include "CompilerExceptions.hpp"
#include <json/json.h>

const std::unordered_map<std::string, bool> SampleRender::Compiler::s_Keywords =
{
	{"AppendStructuredBuffer", false},
	{"BlendState", false},
	{"bool", true},
	{"break", false},
	{"Buffer", false},
	{"ByteAddressBuffer", false},
	{"case", false},
	{"cbuffer", false},
	{"centroid", false},
	{"class", false},
	{"column_major", false},
	{"compile", false},
	{"CompileShader", false},
	{"const", false},
	{"continue", false},
	{"ConsumeStructuredBuffer", false},
	{"default", false},
	{"DepthStencilState", false},
	{"discard", false},
	{"do", false},
	{"double", true},
	{"else", false},
	{"export", false},
	{"extern", false},
	{"false", false},
	{"float", true},
	{"for", false},
	{"GeometryShader", false},
	{"groupshared", false},
	{"half", true},
	{"if", false},
	{"in", false},
	{"inline", false},
	{"inout", false},
	{"InputPatch", false},
	{"int", true},
	{"interface", false},
	{"line", false},
	{"lineadj", false},
	{"linear", false},
	{"LineStream", false},
	{"matrix", false},
	{"min16float", true},
	{"min10float", true},
	{"min16int", true},
	{"min12int", true},
	{"min16uint", true},
	{"namespace", false},
	{"nointerpolation", false},
	{"noperspective", false},
	{"NULL", false},
	{"out", false},
	{"OutputPatch", false},
	{"packoffset", false},
	{"pass", false},
	{"PixelShader", false},
	{"point", false},
	{"PointStream", false},
	{"precise", false},
	{"RasterizerState", false},
	{"return", false},
	{"register", false},
	{"row_major", false},
	{"RWBuffer", false},
	{"RWByteAddressBuffer", false},
	{"RWStructuredBuffer", false},
	{"RWTexture1D", false},
	{"RWTexture1DArray", false},
	{"RWTexture2D", false},
	{"RWTexture2DArray", false},
	{"RWTexture3D", false},
	{"sample", false},
	{"sampler", false},
	{"SamplerState", false},
	{"SamplerComparisonState", false},
	{"shared", false},
	{"snorm", false},
	{"static", false},
	{"string", false},
	{"struct", false},
	{"switch", false},
	{"StructuredBuffer", false},
	{"tbuffer", false},
	{"technique", false},
	{"technique10", false},
	{"technique11", false},
	{"texture", false},
	{"Texture1D", false},
	{"Texture1DArray", false},
	{"Texture2D", false},
	{"Texture2DArray", false},
	{"Texture2DMS", false},
	{"Texture2DMSArray", false},
	{"Texture3D", false},
	{"TextureCube", false},
	{"TextureCubeArray", false},
	{"true", false},
	{"typedef", false},
	{"triangle", false},
	{"triangleadj", false},
	{"TriangleStream", false},
	{"uint", true},
	{"uniform", false},
	{"unorm", false},
	{"unsigned", false},
	{"vector", false},
	{"VertexShader", false},
	{"void", false},
	{"volatile", false},
	{"while", false},
};

SampleRender::Compiler::Compiler(HLSLBackend backend, std::string baseEntry) :
	m_FeatureLevel("_6_7"),
	m_PackedShaders(true),
	m_BaseEntry(baseEntry),
	m_Backend(backend),
	m_DebugMode(true)
{
}

SampleRender::Compiler::~Compiler()
{
}

void SampleRender::Compiler::SetBaseEntry(std::string baseEntry)
{
	m_BaseEntry = baseEntry;
}

void SampleRender::Compiler::PushShaderPath(std::string filepath)
{
	std::regex pattern("^(.*[\\/])([^\\/]+)\\.hlsl$");

	std::smatch matches;

	// Check if the input matches the pattern
	if (std::regex_match(filepath, matches, pattern))
	{
		std::stringstream buffer;
		buffer << matches[1].str();
		buffer << matches[2].str();		
	}
	else
	{
		throw InvalidFilepathException("Invalid filename");
	}
	m_ShaderFilepaths.push_back(filepath);
}

void SampleRender::Compiler::SetBuildMode(bool isDebug)
{
	m_DebugMode = isDebug;
}

void SampleRender::Compiler::CompilePackedShader()
{
	static const std::regex pattern("^(.*[\\/])([^\\/]+)\\.hlsl$");
	std::smatch matches;
	static const std::vector<std::string> shaderStages = { "vs", "ps" };

	Json::Value root;
	Json::StreamWriterBuilder builder;
	const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

	for (auto& shaderPath : m_ShaderFilepaths)
	{
		std::string basepath;
		if (std::regex_match(shaderPath, matches, pattern))
		{
			std::stringstream buffer;
			buffer << matches[1].str();
			buffer << matches[2].str();
			basepath = buffer.str();
			buffer.str("");
			if (!FileHandler::FileExists(shaderPath))
				throw InvalidFilepathException("File not found");
			std::string shader;
			FileHandler::ReadTextFile(shaderPath, &shader);
			for (auto& stage : shaderStages)
			{
				CompileStage(shader, stage, basepath);
				buffer << matches[2].str() << "." << stage << (m_Backend == HLSLBackend::CSO ? ".cso" : ".spv");
				root["BinShaders"][stage] = buffer.str();
				buffer.str("");
			}
			writer->write(root, &buffer);
			std::string jsonResult = buffer.str();
			buffer.str("");
			buffer << basepath << ".json";
			std::string jsonPath = buffer.str();
			buffer.str("");
			FileHandler::WriteTextFile(jsonPath, jsonResult);
		}
	}

	
}

void SampleRender::Compiler::CompileStage(std::string source, std::string stage, std::string basepath)
{
	HRESULT hr;
	std::stringstream buffer;
	std::string tempbuffer;
	std::wstring entrypoint;
	std::wstring formattedStage;
	std::string outputPath;

	buffer << stage << m_BaseEntry;
	tempbuffer = buffer.str();
	ValidateName(tempbuffer);
	entrypoint = std::wstring(tempbuffer.begin(), tempbuffer.end());
	buffer.str("");
	buffer << stage << m_FeatureLevel;
	tempbuffer = buffer.str();
	formattedStage = std::wstring(tempbuffer.begin(), tempbuffer.end());
	buffer.str("");
	buffer << basepath << "." << stage << (m_Backend == HLSLBackend::CSO ? ".cso" : ".spv");
	outputPath = buffer.str();
	buffer.str("");

	ComPointer<IDxcBlob> blob;
	ComPointer<IDxcBlob> errorBlob;
	ComPointer<IDxcUtils> utils;
	ComPointer<IDxcCompiler3> compiler;

	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()));
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.GetAddressOf()));

	std::vector<const wchar_t*> args;
	args.push_back(L"-Zpc");
	args.push_back(L"-HV");
	args.push_back(L"2021");
	args.push_back(L"-T");
	args.push_back(formattedStage.c_str());
	args.push_back(L"-E");
	args.push_back(entrypoint.c_str());
	if(m_DebugMode)
		args.push_back(L"-O0");
	else
		args.push_back(L"-O3");

	DxcBuffer srcBuffer =
	{
		.Ptr = (void *) &*source.begin(),
		.Size = (uint32_t) source.size(),
		.Encoding = 0
	};

	ComPointer<IDxcResult> result;
	hr = compiler->Compile(&srcBuffer, args.data(),(uint32_t) args.size(), nullptr, IID_PPV_ARGS(result.GetAddressOf()));

	blob.Release();
	hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(blob.GetAddressOf()), nullptr);

	if (blob->GetBufferSize() == 0)
	{
		hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errorBlob.GetAddressOf()), nullptr);
		Console::CoreError("DXC Status: {}", (const char*)errorBlob->GetBufferPointer());
		ValidatePipeline(stage);
	}
	else
		FileHandler::WriteBinFile(outputPath,(std::byte*)blob->GetBufferPointer(), blob->GetBufferSize());
}

void SampleRender::Compiler::ValidateName(std::string name)
{
	std::regex variablePattern("[a-zA-Z_][a-zA-Z0-9_]*$");
	std::regex typeExtended("([1-4]|[1-4]x[1-4])$");
	std::regex typeExtendedCapture("^(.*?)([1-4]|[1-4]x[1-4])$");
	std::smatch matches;
	std::string subType;
	bool usesSizer = false;
	if (!std::regex_match(name, matches, variablePattern))
		throw InvalidNameException("The name must start with a letter or \"_\" and can only be followed by a letter a digit or \"_\"");
	if (std::regex_match(name, matches, typeExtended))
	{
		std::regex_match(name, matches, typeExtendedCapture);
		subType = matches[1].str();
		usesSizer = true;
	}
	else
	{
		subType = name;
		usesSizer = false;
	}

	auto it = s_Keywords.find(subType);
	if (it != s_Keywords.end())
		if(it->second == usesSizer)
			throw InvalidNameException("You cannot use HLSL keywords");
}

void SampleRender::Compiler::ValidatePipeline(std::string stage)
{
	std::stringstream buffer;
	if ((stage.compare("ps") == 0) || (stage.compare("vs") == 0))
	{
		buffer << stage << " is a mandatory stage";
		std::string message = buffer.str();
		throw InvalidPipelineException(message);
	}
}
