#include "Compiler.hpp"
#include <regex>
#include "Console.hpp"
#include "FileHandler.hpp"
#include "CompilerExceptions.hpp"
#include <json/json.h>

const std::list<std::pair<uint32_t, uint32_t>> SampleRender::Compiler::s_ValidHLSL =
{
	{6, 0},
	{6, 1},
	{6, 2},
	{6, 3},
	{6, 4},
	{6, 5},
	{6, 6},
	{6, 7},
	{6, 8}
};

const std::list<std::pair<uint32_t, uint32_t>> SampleRender::Compiler::s_ValidVulkan =
{
	{1, 0},
	{1, 1},
	{1, 2},
	{1, 3}
};

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

const std::unordered_map<std::string, bool> SampleRender::Compiler::s_SysValues =
{
	{"CLIPDISTANCE", true},
	{"CULLDISTANCE", true},
	{"COVERAGE", false},
	{"DEPTH", false},
	{"DEPTHGREATEREQUAL", false},
	{"DEPTHLESSEQUAL", false},
	{"DISPATCHTHREADID", false},
	{"DOMAINLOCATION", false},
	{"GROUPID", false},
	{"GROUPINDEX", false},
	{"GROUPTHREADID", false},
	{"GSINSTANCEID", false},
	{"INNERCOVERAGE", false},
	{"INSIDETESSFACTOR", false},
	{"INSTANCEID", false},
	{"ISFRONTFACE", false},
	{"OUTPUTCONTROLPOINTID", false},
	{"POSITION", false},
	{"PRIMITIVEID", false},
	{"RENDERTARGETARRAYINDEX", false},
	{"SAMPLEINDEX", false},
	{"STENCILREF", false},
	{"TARGET", true},
	{"TESSFACTOR", false},
	{"VERTEXID", false},
	{"VIEWPORTARRAYINDEX", false},
	{"SHADINGRATE", false},
};

const std::list<std::string> SampleRender::Compiler::s_BuiltinFunctions =
{
	{"abort"},
	{"abs"},
	{"acos"},
	{"all"},
	{"AllMemoryBarrier"},
	{"AllMemoryBarrierWithGroupSync"},
	{"any"},
	{"asdouble"},
	{"asfloat"},
	{"asin"},
	{"asint"},
	{"asuint"},
	{"asuint"},
	{"atan"},
	{"atan2"},
	{"ceil"},
	{"CheckAccessFullyMapped"},
	{"clamp"},
	{"clip"},
	{"cos"},
	{"cosh"},
	{"countbits"},
	{"cross"},
	{"D3DCOLORtoUBYTE4"},
	{"ddx"},
	{"ddx_coarse"},
	{"ddx_fine"},
	{"ddy"},
	{"ddy_coarse"},
	{"ddy_fine"},
	{"degrees"},
	{"determinant"},
	{"DeviceMemoryBarrier"},
	{"DeviceMemoryBarrierWithGroupSync"},
	{"distance"},
	{"dot"},
	{"dst"},
	{"errorf"},
	{"EvaluateAttributeCentroid"},
	{"EvaluateAttributeAtSample"},
	{"EvaluateAttributeSnapped"},
	{"exp"},
	{"exp2"},
	{"f16tof32"},
	{"f32tof16"},
	{"faceforward"},
	{"firstbithigh"},
	{"firstbitlow"},
	{"floor"},
	{"fma"},
	{"fmod"},
	{"frac"},
	{"frexp"},
	{"fwidth"},
	{"GetRenderTargetSampleCount"},
	{"GetRenderTargetSamplePosition"},
	{"GroupMemoryBarrier"},
	{"GroupMemoryBarrierWithGroupSync"},
	{"InterlockedAdd"},
	{"InterlockedAnd"},
	{"InterlockedCompareExchange"},
	{"InterlockedCompareStore"},
	{"InterlockedExchange"},
	{"InterlockedMax"},
	{"InterlockedMin"},
	{"InterlockedOr"},
	{"InterlockedXor"},
	{"isfinite"},
	{"isinf"},
	{"isnan"},
	{"ldexp"},
	{"length"},
	{"lerp"},
	{"lit"},
	{"log"},
	{"log10"},
	{"log2"},
	{"mad"},
	{"max"},
	{"min"},
	{"modf"},
	{"msad4"},
	{"mul"},
	{"noise"},
	{"normalize"},
	{"pow"},
	{"printf"},
	{"Process2DQuadTessFactorsAvg"},
	{"Process2DQuadTessFactorsMax"},
	{"Process2DQuadTessFactorsMin"},
	{"ProcessIsolineTessFactors"},
	{"ProcessQuadTessFactorsAvg"},
	{"ProcessQuadTessFactorsMax"},
	{"ProcessQuadTessFactorsMin"},
	{"ProcessTriTessFactorsAvg"},
	{"ProcessTriTessFactorsMax"},
	{"ProcessTriTessFactorsMin"},
	{"radians"},
	{"rcp"},
	{"reflect"},
	{"refract"},
	{"reversebits"},
	{"round"},
	{"rsqrt"},
	{"saturate"},
	{"sign"},
	{"sin"},
	{"sincos"},
	{"sinh"},
	{"smoothstep"},
	{"sqrt"},
	{"step"},
	{"tan"},
	{"tanh"},
	{"tex1D"},
	{"tex1D"},
	{"tex1Dbias"},
	{"tex1Dgrad"},
	{"tex1Dlod"},
	{"tex1Dproj"},
	{"tex2D"},
	{"tex2D"},
	{"tex2Dbias"},
	{"tex2Dgrad"},
	{"tex2Dlod"},
	{"tex2Dproj"},
	{"tex3D"},
	{"tex3D"},
	{"tex3Dbias"},
	{"tex3Dgrad"},
	{"tex3Dlod"},
	{"tex3Dproj"},
	{"texCUBE"},
	{"texCUBE"},
	{"texCUBEbias"},
	{"texCUBEgrad"},
	{"texCUBElod"},
	{"texCUBEproj"},
	{"transpose"},
	{"trunc"}
};

SampleRender::Compiler::Compiler(HLSLBackend backend, std::string baseEntry, std::string hlslFeatureLevel, std::wstring vulkanFeatureLevel) :
	m_PackedShaders(true),
	m_BaseEntry(baseEntry),
	m_Backend(backend),
	m_DebugMode(true)
{
	ValidateHLSLFeatureLevel(hlslFeatureLevel);
	m_HLSLFeatureLevel = hlslFeatureLevel;
	ValidateVulkanFeatureLevel(vulkanFeatureLevel);
	m_VulkanFeatureLevel = vulkanFeatureLevel;
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
			root["HLSLFeatureLevel"] = m_HLSLFeatureLevel;
			if (m_Backend == HLSLBackend::SPV) {
				std::string castedFeatureLevel = std::string(m_VulkanFeatureLevel.begin(), m_VulkanFeatureLevel.end());
				root["VulkanFeatureLevel"] = castedFeatureLevel;
			}
			writer->write(root, &buffer);
			std::string jsonResult = buffer.str();
			buffer.str("");
			buffer << basepath << (m_Backend == HLSLBackend::CSO ? ".d3d12" : ".vk") << ".json";
			std::string jsonPath = buffer.str();
			buffer.str("");
			FileHandler::WriteTextFile(jsonPath, jsonResult);
		}
	}
}

void SampleRender::Compiler::SetHLSLFeatureLevel(std::string version)
{
	ValidateHLSLFeatureLevel(version);
	m_HLSLFeatureLevel = version;
}

void SampleRender::Compiler::SetVulkanFeatureLevel(std::wstring version)
{
	ValidateVulkanFeatureLevel(version);
	m_VulkanFeatureLevel = version;
}

void SampleRender::Compiler::ValidateHLSLFeatureLevel(std::string version)
{
	std::regex pattern("^_(\\d+)_(\\d+)$");
	std::smatch matches;
	uint32_t major = 0;
	uint32_t minor = 0;
	if (std::regex_search(version, matches, pattern)) {
		std::stringstream buffer;
		buffer << matches[1].str() << " " << matches[2].str();
		std::istringstream reader(buffer.str());
		reader >> major >> minor;
	}
	else
	{
		throw InvalidHLSLVersion("The HLSL version must match the pattern \"^_(\\d+ )_(\\d+)$\"");
	}
	auto it = SearchHLSLVersion(std::make_pair(major, minor));
	if (it == s_ValidHLSL.end())
	{
		std::stringstream valid_hlsl;
		valid_hlsl << "\"";
		for (auto it = s_ValidHLSL.begin(); it != s_ValidHLSL.end(); it++)
			valid_hlsl << "_" << it->first << "_" << it->second << "|";
		valid_hlsl << "\"";
		throw InvalidHLSLVersion("The valid HLSL versions are: {}");
	}
}

void SampleRender::Compiler::ValidateVulkanFeatureLevel(std::wstring version)
{
	std::wregex pattern(L"^(\\d+)\\.(\\d+)$");
	std::wsmatch matches;
	uint32_t major = 0;
	uint32_t minor = 0;
	if (std::regex_search(version, matches, pattern)) {
		std::wstringstream buffer;
		buffer << matches[1].str() << " " << matches[2].str();
		std::wistringstream reader(buffer.str());
		reader >> major >> minor;
	}
	else
	{
		throw InvalidVulkanVersion("The HLSL version must match the pattern \"^(\\d+)\\.(\\d+)$\"");
	}
	auto it = SearchVulkanVersion(std::make_pair(major, minor));
	if (it == s_ValidVulkan.end())
	{
		std::stringstream valid_vulkan;
		valid_vulkan << "\"";
		for (auto it = s_ValidVulkan.begin(); it != s_ValidVulkan.end(); it++)
			valid_vulkan << "_" << it->first << "_" << it->second << "|";
		valid_vulkan << "\"";
		throw InvalidVulkanVersion("The valid HLSL versions are: {}");
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
	ValidateNameOverKeywords(tempbuffer);
	ValidateNameOverSysValues(tempbuffer);
	ValidateNameOverBuiltinFunctions(tempbuffer);
	entrypoint = std::wstring(tempbuffer.begin(), tempbuffer.end());
	buffer.str("");
	buffer << stage << m_HLSLFeatureLevel;
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

	std::wstring vulkan_fl;
	if (m_Backend == HLSLBackend::SPV)
	{
		std::wstringstream vulkan_fl_buffer;
		vulkan_fl_buffer << L"-fspv-target-env=vulkan" << m_VulkanFeatureLevel;
		vulkan_fl = vulkan_fl_buffer.str();
		args.push_back(L"-spirv");
		args.push_back(vulkan_fl.c_str());
		if((stage.compare("vs") == 0) || (stage.compare("gs") == 0) || (stage.compare("ds") == 0))
			args.push_back(L"-fvk-invert-y");
	}

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

std::list<std::string>::const_iterator SampleRender::Compiler::SearchBuiltinName(std::string name)
{
	for (auto it = s_BuiltinFunctions.begin(); it != s_BuiltinFunctions.end(); it++)
		if (it->compare(name) == 0)
			return it;
	return s_BuiltinFunctions.end();
}

std::list<std::pair<uint32_t, uint32_t>>::const_iterator SampleRender::Compiler::SearchHLSLVersion(std::pair<uint32_t, uint32_t> value)
{
	for (auto it = s_ValidHLSL.begin(); it != s_ValidHLSL.end(); it++)
		if ((it->first == value.first) && (it->second == value.second))
			return it;
	return s_ValidHLSL.end();
}

std::list<std::pair<uint32_t, uint32_t>>::const_iterator SampleRender::Compiler::SearchVulkanVersion(std::pair<uint32_t, uint32_t> value)
{
	for (auto it = s_ValidVulkan.begin(); it != s_ValidVulkan.end(); it++)
		if ((it->first == value.first) && (it->second == value.second))
			return it;
	return s_ValidVulkan.end();
}

void SampleRender::Compiler::ValidateNameOverKeywords(std::string name)
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

void SampleRender::Compiler::ValidateNameOverSysValues(std::string name)
{
	std::regex svPattern("^SV_(\\w+)");
	std::regex enumerationPattern("^(\\D+)(\\d+)$");

	std::smatch matches;
	std::string treatedName;
	bool enumerated = false;

	if (std::regex_search(name, matches, svPattern))
	{
		treatedName = matches[1].str();
	}
	else
	{
		return;
	}

	if (std::regex_search(treatedName, matches, enumerationPattern))
	{
		treatedName = matches[1].str();
		enumerated = true;
	}

	auto it = s_SysValues.find(treatedName);
	if (it != s_SysValues.end())
		if (it->second == enumerated)
		{
			std::stringstream error;
			std::string castedError;
			error << name << " is a HLSL System Value (SV_) and cannot be used";
			castedError = error.str();
			throw InvalidNameException(castedError);
		}
}

void SampleRender::Compiler::ValidateNameOverBuiltinFunctions(std::string name)
{
	auto it = SearchBuiltinName(name);
	if (it != s_BuiltinFunctions.end())
	{
		std::stringstream error;
		std::string castedError;
		error << it->c_str() << " is a builtin function";
		castedError = error.str();
		throw InvalidNameException(castedError);
	}
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
