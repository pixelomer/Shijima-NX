// 
// Shijima-NX - Shimeji desktop pet runner for Nintendo Switch
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <shijima/mascot/factory.hpp>
#include <filesystem>
#include <vector>
#include <sstream>
#include <fstream>
#include <zlib.h>
#include <map>

static unsigned char asciitolower(unsigned char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

static void asciitolower(std::string &data) {
    std::transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c){ return asciitolower(c); });
}

static shijima::mascot::factory gFactory;

struct Asset {
    uint16_t width;
    uint16_t height;
    uint16_t bufsize;
    uint8_t *buf;

    void clear(tsl::gfx::Renderer *renderer, s32 fbx, s32 fby) const {
        for (s32 y=0, ry; y<height && (ry = fby+y)<framebufferHeight; ++y) {
            for (s32 x=0, rx; x<width && (rx = fbx+x)<framebufferWidth; ++x) {
                renderer->setPixel(rx, ry, 0);
            }
        }
    }

    void draw(tsl::gfx::Renderer *renderer, s32 fbx, s32 fby, bool flipped) const {
        uint8_t decBuf[0x1000];
        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = bufsize;
        stream.next_in = (Bytef *)buf;
        stream.avail_out = sizeof(decBuf);
        stream.next_out = (Bytef *)decBuf;
        int ret = inflateInit(&stream);
        if (ret != Z_OK) {
            throw std::runtime_error("inflateInit() failed: " + std::to_string(ret));
        }
        size_t off = 0;
        for (s32 y=0, ry; y<height && (ry = fby+y)<framebufferHeight; ++y) {
            for (s32 x=0; x<width; ++x) {
                if (stream.next_out == (Bytef *)decBuf && stream.avail_in != 0) {
                    off = stream.total_out;
                    ret = inflate(&stream, Z_SYNC_FLUSH);
                    if (ret != Z_STREAM_END && ret != Z_OK) {
                        inflateEnd(&stream);
                        throw std::runtime_error("inflate() error: " + std::to_string(ret));
                    }
                }
                stream.avail_out += 2;
                stream.next_out -= 2;
                s32 rx = fbx;
                if (flipped) {
                    rx += width - x - 1;
                }
                else {
                    rx += x;
                }
                if (rx<framebufferWidth) {
                    uint8_t *pixel = &decBuf[y*width*2 + x*2 - off];
                    tsl::gfx::Color color { (u8)(pixel[0] >> 4),
                        (u8)(pixel[0] & 0xF),
                        (u8)(pixel[1] >> 4),
                        (u8)(pixel[1] & 0xF) };
                    renderer->setPixelBlendDst(rx, ry, renderer->a(color));
                }
            }
        }
        inflateEnd(&stream);
    }
};

class AssetPack {
public:
    AssetPack(): m_buf(nullptr) {}
    void load(std::filesystem::path path) {
        clear();

        // read file into buffer
        m_size = std::filesystem::file_size(path);
        m_buf = new uint8_t[m_size];
        {
            std::ifstream file;
            file.open(path, std::ios::binary);
            file.read((char *)m_buf, m_size);
            if (!file.good()) {
                delete[] m_buf;
                throw std::runtime_error("read failed");
            }
            else if (file.get(), !file.eof()) {
                delete[] m_buf;
                throw std::runtime_error("size mismatch");
            }
            file.close();
        }
        
        // scan file
        //WONTFIX: potential out-of-bounds read with crafted input
        for (size_t i=0; i<m_size; ) {
            #define word() (i += 2, (((uint16_t)((m_buf)[i-2]) << 8) | ((m_buf)[i-1])))
            Asset asset;
            asset.width = word();
            asset.height = word();
            uint16_t nameLen = word();
            std::string name { (const char *)m_buf + i, nameLen };
            asciitolower(name);
            i += nameLen;
            asset.bufsize = word();
            asset.buf = m_buf + i;
            i += asset.bufsize;
            m_assets[name] = asset;
            #undef word
        }
    }
    void clear() {
        m_assets.clear();
        m_size = 0;
        if (m_buf != nullptr) {
            delete[] m_buf;
            m_buf = nullptr;
        }
    }
    const Asset *image(std::string const& name) {
        auto stem = (std::filesystem::path { name }).stem().string();
        if (m_assets.count(stem) == 1) {
            return &m_assets.at(stem);
        }
        else {
            return nullptr;
        }
    }
private:
    std::map<std::string, Asset> m_assets;
    size_t m_size;
    uint8_t *m_buf;
};

class NXMascot {
public:
    NXMascot(): m_valid(false) {}
    NXMascot(shijima::mascot::factory::product product, AssetPack *assets):
        m_valid(true), m_product(std::move(product)), m_assets(assets),
        m_prevAsset(nullptr) {}
    bool valid() const {
        return m_valid;
    }
    void clear(tsl::gfx::Renderer *renderer) {
        if (m_prevAsset != nullptr) {
            m_prevAsset->clear(renderer, m_prevX, m_prevY);
        }
    }
    void draw(tsl::gfx::Renderer *renderer) {
        auto &mascot = *m_product.manager;
        auto anchor = mascot.state->anchor;
        auto pos = anchor;
        auto &frame = mascot.state->active_frame;
        bool mirroredRender = mascot.state->looking_right &&
            frame.right_name.empty();
        auto name = frame.get_name(mascot.state->looking_right);
        asciitolower(name);
        auto sprite = m_assets->image(name);
        m_prevAsset = sprite;
        if (sprite == NULL) {
            return;
        }
        bool flip;
        if (mirroredRender) {
            flip = true;
            pos = { pos.x - (sprite->width - frame.anchor.x), pos.y - frame.anchor.y };
        }
        else {
            flip = false;
            pos = { pos.x - frame.anchor.x, pos.y - frame.anchor.y };
        }
        m_prevX = (s32)pos.x;
        m_prevY = (s32)pos.y;
        sprite->draw(renderer, (s32)pos.x, (s32)pos.y, flip);
    }
    void tick() {
        auto &mascot = *m_product.manager;
        mascot.tick();
    }
    shijima::mascot::manager &manager() {
        return *m_product.manager;
    }
    bool pointInside(u32 x, u32 y) {
        return m_prevAsset != nullptr && (s32)x >= m_prevX &&
            (s32)y >= m_prevY && (s32)x < (m_prevX + m_prevAsset->width) &&
            (s32)y < (m_prevY + m_prevAsset->height);
    }
private:
    bool m_valid;
    shijima::mascot::factory::product m_product;
    AssetPack *m_assets;
    const Asset *m_prevAsset;
    s32 m_prevX, m_prevY;
};

static void registerTemplate(std::filesystem::path cerealPath) {
    std::ostringstream stream;
    std::ifstream file;
    file.open(cerealPath, std::ios::binary);
    stream << file.rdbuf();
    file.close();
    shijima::mascot::factory::registered_tmpl tmpl;
    tmpl.data = stream.str();
    stream = {};
    tmpl.name = "mascot";
    gFactory.register_template(tmpl);
}

class ShijimaElement : public tsl::elm::Element {
public:
    ShijimaElement(): tsl::elm::Element() {
        try {
            registerTemplate("/config/Shijima-NX/mascot.cereal");
            m_pack.load("/config/Shijima-NX/img.bin");
            auto product = gFactory.spawn("mascot");
            product.manager->state->anchor = { 100, 100 };
            m_mascots.push_back(new NXMascot { std::move(product), &m_pack });
        }
        catch (std::exception &ex) {
            m_error = ex.what();
        }
    }
    virtual void draw(tsl::gfx::Renderer *renderer) override {
        if (m_error.empty()) {
            for (auto mascot : m_mascots) {
                mascot->clear(renderer);
            }
            for (auto mascot : m_mascots) {
                try {
                    mascot->draw(renderer);
                }
                catch (std::exception &ex) {
                    m_error = ex.what();
                }
            }
        }
        else {
            renderer->drawRect(0, 0, framebufferWidth, 130, { 0xFF, 0, 0, 0xFF });
            renderer->drawString("A fatal error occurred and Shijima-NX cannot continue.",
                false, 50, 50, 20, 0xFFFF);
            renderer->drawString("Press L-DPadDown-RStick to return to Tesla-Menu.",
                false, 50, 70, 20, 0xFFFF);
            renderer->drawString(m_error.c_str(), false, 50, 90, 20, 0xFFFF);
        }
    }
    virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
        setBoundaries(0, 0, framebufferWidth, 720);
    }
    NXMascot *hitTest(u32 x, u32 y) {
        for (int i=(int)m_mascots.size()-1; i>=0; --i) {
            auto mascot = m_mascots[i];
            if (mascot->pointInside(x, y)) {
                return mascot;
            }
        }
        return nullptr;
    }
    void tick() {
        for (auto mascot : m_mascots) {
            mascot->tick();
        }
    }
private:
    std::vector<NXMascot *> m_mascots;
    AssetPack m_pack;
    std::string m_error;
};

class ShijimaGui : public tsl::Gui {
public:
    ShijimaGui(): m_shijimaElement(new ShijimaElement) {}

    // Called when this Gui gets loaded to create the UI
    // Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
    virtual tsl::elm::Element* createUI() override {
        return m_shijimaElement;
    }

    // Called once every frame to update values
    virtual void update() override {
        auto &env = *gFactory.env;
        env.screen = { 0, (double)framebufferWidth, (double)framebufferHeight, 0 };
        env.work_area = env.screen;
        env.ceiling = { 0, 0, (double)framebufferWidth };
        env.floor = { (double)framebufferHeight, 0, (double)framebufferWidth };
        env.subtick_count = 1;
        env.allows_breeding = false;
        m_shijimaElement->tick();
        gFactory.env->cursor.dx = gFactory.env->cursor.dy = 0;
    }

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        constexpr u64 backCombo = KEY_L | KEY_DDOWN | KEY_RSTICK;
        if (touchPos.x != 0) {
            gFactory.env->cursor.move({ (double)touchPos.x / layerScale,
                (double)touchPos.y / layerScale });
            if (!m_touchDown) {
                m_touchDown = true;
                auto mascot = m_shijimaElement->hitTest((u32)(touchPos.x / layerScale),
                    (u32)(touchPos.y / layerScale));
                if (mascot != nullptr) {
                    m_draggedMascot = mascot->manager().state;
                    m_draggedMascot->dragging = true;
                }
            }
        }
        else if (m_touchDown) {
            m_touchDown = false;
            if (m_draggedMascot != nullptr) {
                m_draggedMascot->dragging = false;
            }
            m_draggedMascot = nullptr;
        }
        if ((keysHeld & backCombo) == backCombo) {
            tsl::goBack();
            return true;
        }
        if (m_touchDown && m_draggedMascot != nullptr) {
            return true;
        }
        return false; // Return true here to signal the inputs have been consumed
    }
private:
    bool m_touchDown = false;
    std::shared_ptr<shijima::mascot::state> m_draggedMascot;
    ShijimaElement *m_shijimaElement;
};

class ShijimaOverlay : public tsl::Overlay {
public:
    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<ShijimaGui>(); 
    }
};

int main(int argc, char **argv) {
    gFactory.env = std::make_shared<shijima::mascot::environment>();
    layerScale = 1.25;
    framebufferWidth = (u16)(tsl::cfg::LayerMaxWidth / layerScale);
    framebufferHeight = (u16)(tsl::cfg::LayerMaxHeight / layerScale);
    return tsl::loop<ShijimaOverlay>(argc, argv);
}
