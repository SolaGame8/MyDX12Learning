
cbuffer MyConstants : register(b0)
{
    float4 shaderParam[8]; // float4��8�i�[
    
    matrix worldMat;
    matrix viewMat;
    matrix projMat;
    
    /*
    //���Ƃ��Α��z���Ȃǂ��v�Z�������ꍇ�͂��̂悤�ɁA���z�̌����̃x�N�g�����󂯎�����肵�܂�
    float4 lightDirection;
    */
    
};

Texture2D texArray[3] : register(t0); // �����̃e�N�X�`����z��Ƃ��Ē�`   //0 chara, 1 map
SamplerState smp : register(s0);        //�T���v���[�i�摜�̐F���擾����j

// �V�F�[�_�[�̓��͍\����
struct VertexIn
{
    float4 pos : POSITION; // ���_�̈ʒu���
    float4 color : COLOR; // ���_�̐F���
    float4 uv : TEXCOORD; // UV���W���
    /*
    //���Ƃ��΁A���C�g�̏����������ꍇ�́A�@����������Ȋ����Ŏ󂯎�����肵�܂�
    //float4 normal : NORMAL;   //�ʂ̌����̃x�N�g��
    //�����̂悤�Ɏg�p�������ꍇ�́A�n������`�惋�[�������ׂĕύX���܂�
    */
    
    uint instanceId : SV_InstanceID; // �C���X�^���XID��ǉ� 0-7
};

// �V�F�[�_�[�̏o�͍\����
struct VertexOut
{
    float4 pos : SV_POSITION; // ���_�̍ŏI�I�Ȉʒu�i�X�N���[�����W�n�j
    float4 color : COLOR; // ��Ԃ��ꂽ�F���
    float2 uv : TEXCOORD; // ��Ԃ��ꂽUV���W
};

// ���_�V�F�[�_�[ (Vertex Shader)
// �e���_�̃f�[�^���󂯎��A�X�N���[�����W�֕ϊ�
VertexOut VSMain(VertexIn vin)
{
    VertexOut vout;

    
    float4 pos = vin.pos;
    
    
    //�J�����̕ϊ��s��
    pos = mul(worldMat, pos);
    pos = mul(viewMat, pos);
    pos = mul(projMat, pos);
    
    
    
    vout.pos = pos;
    vout.color = vin.color;
    vout.uv = vin.uv.xy;

    return vout;
}

// �s�N�Z���V�F�[�_�[ (Pixel Shader)
// �e�s�N�Z���̍ŏI�I�ȐF������
float4 PSMain(VertexOut pin) : SV_TARGET        //���X�^���C�Y����ď�񂪓n�����
{
    // ���_�V�F�[�_�[�����Ԃ���ēn���ꂽ�F�����̂܂܏o��
    
    float4 col = texArray[1].Sample(smp, pin.uv.xy);
    
    /*
    //���Ƃ��΁A�����ő��z���Ȃǂ��v�Z�������ꍇ�͂���Ȋ����ɂ��܂�
    
    float3 n_lightDirection = normalize(lightDirection.xyz); //���z�̃x�N�g�����m�[�}���C�Y�i����1�̃x�N�g���ɂ���j�@���Ƃ��ƃm�[�}���C�Y���ēn�������������ł�
    float3 n_normal = normalize(pin.normal.xyz);      //������ŏ�����m�[�}���C�Y����������������������ł�
    
    float lightDotValue = dot(n_lightDirection.xyz, n_normal.xyz); //���ς��v�Z //.xyz�͕s�v�ł����C���[�W���₷�������Ȃ̂ŏ����Ă��܂�
    
    //����lightDotValue�̌��ʂ́A1.0 �` -1.0�ɂȂ�܂�
    
    //���ς́A�Q�̃x�N�g�����A���������������Ă��鎞���A1.0
    //90�x�݂����Ȏ��́A0.0
    //�t�����������Ă��鎞�́A-1.0
    
    //�Ȃ̂ŁA-1.0�̎����A���z�������Ă�������ɁA���������Ă���ʂł��i���݂��t�����j
    
    //����Ȋ����ŐF��ω��������܂�
    
    float lightStrings = 0.8f;    //���C�g�̋���  �����̒l���ϐ����Ƃ��Ď󂯎���������g�����肪�ǂ��ł�
    float ambientLightVal = 0.2f; //���̖��邳�i�����0.0f�ɂ���ƁA�e���^�����ɂȂ�܂��j
    
    float l_dot = saturate(-lightDotValue);    //saturate�́A�w�肵���l�� 0 ���� 1 �͈͓̔��ŃN�����v���܂�
    col = col * l_dot * lightStrings + col * ambientLightVal;
    
    */
    
    
    return col;
        

}

