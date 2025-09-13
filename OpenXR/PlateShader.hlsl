

cbuffer MyConstants : register(b0)  //b0�ɓo�^���������擾
{

    float4 shaderParam[8]; // float4��8�i�[
    
        //[0]   �L�����N�^�[�̈ʒu             x,y,z
        //[1]   �L�����N�^�[�̕`�悷��UV�̈ʒu   u,v,���_id
    
        //[2]   ���u�̈ʒu             x,y,z
        //[3]   ���u�̕`�悷��UV�̈ʒu   u,v,���_id
        
        //[4]   �R�C���̈ʒu             x,y,z
        //[5]   �R�C���̕`�悷��UV�̈ʒu   u,v,���_id
    
        //��3D��Ԃł̈ʒu�́A���̂悤��float4�ł͂Ȃ��Amatrix�i�ʒu�A��]�A�X�P�[�������킳�������́j�ŏ���n�������������ł�
        //���̏����AworldMat�Ƃ��Ĉ��������ł�
    
    matrix worldMat;    //�s�g�p�i�ω��Ȃ��s��j�@3D��Ԃł̃L�����N�^�[�̈ړ���]�ȂǂɎg��
    matrix viewMat;     //�J�����̃r���[�s��
    matrix projMat;     //�J�����̃v���W�F�N�V�����s��
    
    /*
    ����CBV �萔�o�b�t�@�i�ϐ����j�́A16�o�C�g���ӎ��������������ł��i16�o�C�g �A���C�����g�j
    �������邱�ƂŁA�V�F�[�_�[�ɂ�����Ə�񂪓n�����悤�ɂȂ�܂�
    �����ŏ������邽�߂ɁA���̂悤�ȃ��[��������悤�ł�
    
    ���Ƃ��΁A
    float4 shaderParam[8];  //float�i4�o�C�g�j* 4 * 8 = 128�i16�o�C�g��8�{�j
    matrix worldMat;        //float�i4�o�C�g�j* 16�i4x4�s��j = 64�i16�o�C�g��4�{�j 

    �Ȃ̂ŁA���������܂��V�F�[�_�[�ɒl���n����Ȃ��ȁ`���Ďv������`�F�b�N���Ă݂Ă�������
    */
    
};

/*
//���Ƃ��΁Ab1�ɓo�^���������󂯎�肽���ꍇ�́A����Ȋ����ł��Bb2�ȍ~�����l�ł�
cbuffer MyConstants : register(b1)
{
    float4 anotherInfo[16];
};
*/



Texture2D texArray[2] : register(t0); // �����̃e�N�X�`����z��Ƃ��Ē�`�B�@�z��Ȃ̂ŁAt0-t1�ɓo�^���ꂽ�����擾

/*
//���Ƃ��΁At2�ɓo�^���������擾�������ꍇ�͂���Ȋ����ł�
Texture2D anotherTexture : register(t2);
*/



SamplerState smp : register(s0);        //�T���v���[�i�摜�̐F���擾����j  s0�ɓo�^���ꂽ�����擾

/*
//���Ƃ��΁As1�ɓo�^���������擾�������ꍇ�͂���Ȋ����ł�
SamplerState smp : register(s1);
*/




// �V�F�[�_�[�̓��͍\����
struct VertexIn
{
    float4 pos : POSITION; // ���_�̈ʒu���
    float4 color : COLOR; // ���_�̐F���
    float4 uv : TEXCOORD; // UV���W���
    
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
    
    // �C���X�^���XID���擾
    uint instanceId = vin.instanceId;   //0-7

    float4 col = vin.color;
    if (instanceId == 1)
    {
        col.r = 1.0f;

    }
    
    
    float4 pos = vin.pos;

    pos.xyz += shaderParam[instanceId * 2].xyz;     //float4��x,y,z�̐����ɂ��ꂼ���x,y,z�̐����𑫂��Ă��܂��Brgb�Ƃ����Ă�xyz�Ɠ������ʂɂȂ�܂��B

    int id = (int)vin.uv.z; //0123�@���_��id
    
    float2 uv = shaderParam[instanceId * 2 + 1].xy; //����̒��_�̎��́A�e�N�X�`���[�̎w�肳�ꂽ�ʒu
    //�����float4��x,y�̐������Afloat2��x,y�ɓ���Ă��܂��B���Ȃ݂�float4�̐����́Axyzw, rgba�Ȃǎg���₷�����Ŏ擾�ł��܂�
    
    if (id == 1)                                    //�E��̒��_��������A�e�N�X�`���[�̎w�肳�ꂽ�ʒu����A������ �E�ɂ��炷
    {
        uv.x += shaderParam[instanceId * 2 + 1].z;  //�e�N�X�`���[��1�u���b�N��
    }
    else if (id == 2)                               //�����̒��_��������A�e�N�X�`���[�̎w�肳�ꂽ�ʒu����A�c���� ���ɂ��炷
    {
        uv.y += shaderParam[instanceId * 2 + 1].w;  //�e�N�X�`���[��1�u���b�N����
    }
    else if (id == 3)                               //�E���̒��_��������A�e�N�X�`���[�̎w�肳�ꂽ�ʒu����A�E���ɂ��炷
    {
        uv.x += shaderParam[instanceId * 2 + 1].z;  //1�u���b�N��
        uv.y += shaderParam[instanceId * 2 + 1].w;  //1�u���b�N����
    }
    
    
    
    //�J�����̕ϊ��s��i�|���鏇�Ԓ��Ӂj
    pos = mul(worldMat, pos);
    pos = mul(viewMat, pos);
    pos = mul(projMat, pos);
    
    
    //�s�N�Z���V�F�[�_�[�ɓn���l
    vout.pos = pos;     //���_�ʒu
    vout.color = col;   //�F�i���ݕs�g�p�j
    vout.uv = uv;       //UV

    return vout;
}


// �s�N�Z���V�F�[�_�[ (Pixel Shader)
// �e�s�N�Z���̍ŏI�I�ȐF������
float4 PSMain(VertexOut pin) : SV_TARGET        //���X�^���C�Y����ď�񂪓n�����
{


    float4 col = texArray[0].Sample(smp, pin.uv);   //t0�ɓo�^�����e�N�X�`���[����As0�ɓo�^�����T���v���[���ismp�j���g���āAuv�̈ʒu�̐F�����擾
    
    return col; //�F�����o�́B���̐F�������_�[�^�[�Q�b�g�ɕ`�悳���B
    
    /*
    //���Ƃ��΁Areturn col;����Ȃ��� ���̂悤�ɕԂ��ƁA�Ԃ��Ȃ�܂�
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
    */
}

