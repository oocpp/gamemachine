in vec3 _cubemap_uv;

subroutine (GM_TechniqueEntrance) void GM_CubeMap()
{
    _frag_color = texture(GM_CubeMapTextureAttribute, _cubemap_uv);
}