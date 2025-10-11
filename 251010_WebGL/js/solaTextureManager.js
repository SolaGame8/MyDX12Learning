// solaTextureManager.js

/**
 * WebGLTextureのロードと管理を一手に担うクラス。
 * テクスチャオブジェクトをキー文字列で内部のMapに保持する。
 */


class SolaTextureManager {

    /**
     * @param {WebGLRenderingContext} gl - WebGLコンテキスト
     */

    /**
     * @param {SolaWGL} wglHelper - SolaWGLのインスタンス
     */

    constructor(wglHelper) {
        this.gl = wglHelper.gl;
        // キー文字列(string)とWebGLTextureオブジェクトを紐づけるMap
        this.textures = new Map();
    }

    /**
     * 画像ファイルをロードし、WebGLTextureを作成してMapに登録する。
     * @param {string} key - Mapに登録するテクスチャのキー文字列
     * @param {string} url - ロードする画像ファイルのURL
     * @returns {Promise<WebGLTexture>} - ロードおよび登録されたWebGLTextureオブジェクト
     */
    async loadAndRegister(key, url) {

        const gl = this.gl;


        // 既に同じキーで登録されている場合は、既存のテクスチャを返す
        if (this.textures.has(key)) {
            console.warn(`Texture key "${key}" is already registered. Returning existing texture.`);
            return this.textures.get(key);
        }

        try {
            // 1. 画像ファイルをロード
            const image = await this._loadImage(url);
            
            // 2. WebGLTextureを作成・設定
            const texture = this._createWebGLTexture(gl, image);

            // 3. Mapに登録
            this.textures.set(key, texture);
            
            console.log(`Texture "${key}" loaded and registered.`);

            return true;
        } catch (error) {
            console.error(error.message);
            // ロード失敗時は null を返す
            return false;
        }
    }

    /**
     * 指定されたキーのWebGLTextureオブジェクトを取得する。
     * @param {string} key - 取得したいテクスチャのキー文字列
     * @returns {WebGLTexture | undefined} - WebGLTextureオブジェクト、または存在しない場合はundefined
     */
    getTexture(key) {
        return this.textures.get(key);
    }

    /**
     * テクスチャをMapから削除し、WebGLリソースを解放する。
     * @param {string} key - 削除したいテクスチャのキー文字列
     */
    delete(key) {
        const texture = this.textures.get(key);
        if (texture) {
            this.gl.deleteTexture(texture);
            this.textures.delete(key);
            console.log(`Texture "${key}" deleted and resource freed.`);
        }
    }

    /**
     * 画像ファイルの非同期ロード処理
     * @param {string} url - 画像ファイルのURL
     * @returns {Promise<HTMLImageElement>}
     */
    _loadImage(url) {
        return new Promise((resolve, reject) => {
            const image = new Image();
            // クロスオリジン画像を扱う場合のために、CORSを設定
            image.crossOrigin = 'anonymous'; 
            
            image.onload = () => resolve(image);
            image.onerror = () => reject(new Error(`Failed to load image from ${url}.`));
            image.src = url;
        });
    }
    
    /**
     * HTMLImageElementからWebGLTextureを作成し、パラメータを設定する。
     */
    _createWebGLTexture(gl, image) {
        const texture = gl.createTexture();
        
        // テクスチャユニット0にバインド（一時的な作業用）
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texture);
        
        // 画像をテクスチャとしてアップロード
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
        
        // 画像サイズが2のべき乗かチェック
        const isPowerOf2 = (value) => (value & (value - 1)) === 0;
        
        if (isPowerOf2(image.width) && isPowerOf2(image.height)) {
            // 2のべき乗の場合: ミップマップを使用
            gl.generateMipmap(gl.TEXTURE_2D);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        } else {
            // 2のべき乗ではない場合: CLAMP_TO_EDGEを設定し、ミップマップは使用しない
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        }

        // バインド解除
        gl.bindTexture(gl.TEXTURE_2D, null);
        
        return texture;
    }



    onDestroy() {



    }


}



