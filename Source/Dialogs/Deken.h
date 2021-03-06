#pragma once
#include "../Utility/JSON.h"

struct MD5 {
    MD5(String const& input)
    {
        processBlock(input.toRawUTF8(), input.getNumBytesAsUTF8());
    }

    inline static String encode(String const& input)
    {
        auto encoded = MD5(input);
        return encoded.getResult();
    }

    String getResult() noexcept
    {
        uint8_t encodedLength[8];
        copyWithEndiannessConversion(encodedLength, count, 8);

        // Pad out to 56 mod 64.
        auto index = (count[0] >> 3) & 0x3f;
        auto paddingLength = (index < 56 ? 56 : 120) - index;

        uint8_t paddingBuffer[64] = { 0x80 }; // first byte is 0x80, remaining bytes are zero.

        processBlock(paddingBuffer, (size_t)paddingLength);
        processBlock(encodedLength, 8);

        uint8 result[16];

        copyWithEndiannessConversion(result, state, 16);

        return String::toHexString(result, sizeof(result), 0);
    }

private:
    void processBlock(void const* data, size_t dataSize) noexcept
    {
        auto bufferPos = ((count[0] >> 3) & 0x3f);

        count[0] += (uint32_t)(dataSize << 3);

        if (count[0] < ((uint32_t)dataSize << 3))
            count[1]++;

        count[1] += (uint32_t)(dataSize >> 29);

        auto spaceLeft = (size_t)64 - (size_t)bufferPos;
        size_t i = 0;

        if (dataSize >= spaceLeft) {
            memcpy(buffer + bufferPos, data, spaceLeft);
            transform(buffer);

            for (i = spaceLeft; i + 64 <= dataSize; i += 64)
                transform(static_cast<char const*>(data) + i);

            bufferPos = 0;
        }

        memcpy(buffer + bufferPos, static_cast<char const*>(data) + i, dataSize - i);
    }

    void transform(void const* bufferToTransform) noexcept
    {
        auto a = state[0];
        auto b = state[1];
        auto c = state[2];
        auto d = state[3];

        uint32_t x[16];
        copyWithEndiannessConversion(x, bufferToTransform, 64);

        enum Constants {
            S11 = 7,
            S12 = 12,
            S13 = 17,
            S14 = 22,
            S21 = 5,
            S22 = 9,
            S23 = 14,
            S24 = 20,
            S31 = 4,
            S32 = 11,
            S33 = 16,
            S34 = 23,
            S41 = 6,
            S42 = 10,
            S43 = 15,
            S44 = 21
        };

        FF(a, b, c, d, x[0], S11, 0xd76aa478);
        FF(d, a, b, c, x[1], S12, 0xe8c7b756);
        FF(c, d, a, b, x[2], S13, 0x242070db);
        FF(b, c, d, a, x[3], S14, 0xc1bdceee);
        FF(a, b, c, d, x[4], S11, 0xf57c0faf);
        FF(d, a, b, c, x[5], S12, 0x4787c62a);
        FF(c, d, a, b, x[6], S13, 0xa8304613);
        FF(b, c, d, a, x[7], S14, 0xfd469501);
        FF(a, b, c, d, x[8], S11, 0x698098d8);
        FF(d, a, b, c, x[9], S12, 0x8b44f7af);
        FF(c, d, a, b, x[10], S13, 0xffff5bb1);
        FF(b, c, d, a, x[11], S14, 0x895cd7be);
        FF(a, b, c, d, x[12], S11, 0x6b901122);
        FF(d, a, b, c, x[13], S12, 0xfd987193);
        FF(c, d, a, b, x[14], S13, 0xa679438e);
        FF(b, c, d, a, x[15], S14, 0x49b40821);

        GG(a, b, c, d, x[1], S21, 0xf61e2562);
        GG(d, a, b, c, x[6], S22, 0xc040b340);
        GG(c, d, a, b, x[11], S23, 0x265e5a51);
        GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);
        GG(a, b, c, d, x[5], S21, 0xd62f105d);
        GG(d, a, b, c, x[10], S22, 0x02441453);
        GG(c, d, a, b, x[15], S23, 0xd8a1e681);
        GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);
        GG(a, b, c, d, x[9], S21, 0x21e1cde6);
        GG(d, a, b, c, x[14], S22, 0xc33707d6);
        GG(c, d, a, b, x[3], S23, 0xf4d50d87);
        GG(b, c, d, a, x[8], S24, 0x455a14ed);
        GG(a, b, c, d, x[13], S21, 0xa9e3e905);
        GG(d, a, b, c, x[2], S22, 0xfcefa3f8);
        GG(c, d, a, b, x[7], S23, 0x676f02d9);
        GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);

        HH(a, b, c, d, x[5], S31, 0xfffa3942);
        HH(d, a, b, c, x[8], S32, 0x8771f681);
        HH(c, d, a, b, x[11], S33, 0x6d9d6122);
        HH(b, c, d, a, x[14], S34, 0xfde5380c);
        HH(a, b, c, d, x[1], S31, 0xa4beea44);
        HH(d, a, b, c, x[4], S32, 0x4bdecfa9);
        HH(c, d, a, b, x[7], S33, 0xf6bb4b60);
        HH(b, c, d, a, x[10], S34, 0xbebfbc70);
        HH(a, b, c, d, x[13], S31, 0x289b7ec6);
        HH(d, a, b, c, x[0], S32, 0xeaa127fa);
        HH(c, d, a, b, x[3], S33, 0xd4ef3085);
        HH(b, c, d, a, x[6], S34, 0x04881d05);
        HH(a, b, c, d, x[9], S31, 0xd9d4d039);
        HH(d, a, b, c, x[12], S32, 0xe6db99e5);
        HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
        HH(b, c, d, a, x[2], S34, 0xc4ac5665);

        II(a, b, c, d, x[0], S41, 0xf4292244);
        II(d, a, b, c, x[7], S42, 0x432aff97);
        II(c, d, a, b, x[14], S43, 0xab9423a7);
        II(b, c, d, a, x[5], S44, 0xfc93a039);
        II(a, b, c, d, x[12], S41, 0x655b59c3);
        II(d, a, b, c, x[3], S42, 0x8f0ccc92);
        II(c, d, a, b, x[10], S43, 0xffeff47d);
        II(b, c, d, a, x[1], S44, 0x85845dd1);
        II(a, b, c, d, x[8], S41, 0x6fa87e4f);
        II(d, a, b, c, x[15], S42, 0xfe2ce6e0);
        II(c, d, a, b, x[6], S43, 0xa3014314);
        II(b, c, d, a, x[13], S44, 0x4e0811a1);
        II(a, b, c, d, x[4], S41, 0xf7537e82);
        II(d, a, b, c, x[11], S42, 0xbd3af235);
        II(c, d, a, b, x[2], S43, 0x2ad7d2bb);
        II(b, c, d, a, x[9], S44, 0xeb86d391);

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
    }

    uint8_t buffer[64] = {};
    uint32_t state[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
    uint32_t count[2] = {};

    static void copyWithEndiannessConversion(void* output, void const* input, size_t numBytes) noexcept
    {
#if JUCE_LITTLE_ENDIAN
        memcpy(output, input, numBytes);
#else
        auto dst = static_cast<uint8_t*>(output);
        auto src = static_cast<uint8_t const*>(input);

        for (size_t i = 0; i < numBytes; i += 4) {
            dst[i + 0] = src[i + 3];
            dst[i + 1] = src[i + 2];
            dst[i + 2] = src[i + 1];
            dst[i + 3] = src[i + 0];
        }
#endif
    }

    static uint32_t rotateLeft(uint32_t x, uint32_t n) noexcept { return (x << n) | (x >> (32 - n)); }

    static uint32_t F(uint32_t x, uint32_t y, uint32_t z) noexcept { return (x & y) | (~x & z); }
    static uint32_t G(uint32_t x, uint32_t y, uint32_t z) noexcept { return (x & z) | (y & ~z); }
    static uint32_t H(uint32_t x, uint32_t y, uint32_t z) noexcept { return x ^ y ^ z; }
    static uint32_t I(uint32_t x, uint32_t y, uint32_t z) noexcept { return y ^ (x | ~z); }

    static void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft(a + F(b, c, d) + x + ac, s) + b;
    }

    static void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft(a + G(b, c, d) + x + ac, s) + b;
    }

    static void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft(a + H(b, c, d) + x + ac, s) + b;
    }

    static void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft(a + I(b, c, d) + x + ac, s) + b;
    }
};

struct Spinner : public Component
    , public Timer {
    bool isSpinning = false;

    void startSpinning()
    {
        setVisible(true);
        startTimer(20);
    }

    void stopSpinning()
    {
        setVisible(false);
        stopTimer();
    }

    void timerCallback() override
    {
        repaint();
    }

    void paint(Graphics& g) override
    {
        getLookAndFeel().drawSpinningWaitAnimation(g, findColour(PlugDataColour::textColourId), 3, 3, getWidth() - 6, getHeight() - 6);
    }
};

// Struct with info about the deken package
struct PackageInfo {
    PackageInfo(String name, String author, String timestamp, String url, String description, String version, StringArray objects)
    {
        this->name = name;
        this->author = author;
        this->timestamp = timestamp;
        this->url = url;
        this->description = description;
        this->version = version;
        this->objects = objects;
        packageId = Base64::toBase64(name + "_" + version + "_" + timestamp + "_" + author);
    }

    // fast compare by ID
    friend bool operator==(PackageInfo const& lhs, PackageInfo const& rhs)
    {
        return lhs.packageId == rhs.packageId;
    }

    String name, author, timestamp, url, description, version, packageId;
    StringArray objects;
};

// Array with package info to store the result of a search action in
using PackageList = Array<PackageInfo>;

using namespace nlohmann;

struct PackageSorter {
    static void sort(ValueTree& packageState)
    {
        PackageSorter sorter;
        packageState.sort(sorter, nullptr, true);
    }

    static int compareElements(ValueTree const& first, ValueTree const& second)
    {
        return first.getType().toString().compare(second.getType().toString());
    }
};

struct PackageManager : public Thread
    , public ActionBroadcaster
    , public ValueTree::Listener
    , public DeletedAtShutdown {

    struct DownloadTask : public Thread {
        PackageManager& manager;
        PackageInfo packageInfo;

        std::unique_ptr<InputStream> instream;

        DownloadTask(PackageManager& m, PackageInfo& info)
            : Thread("Download Thread")
            , manager(m)
            , packageInfo(info)
        {
            int statusCode = 0;
            instream = URL(info.url).createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress)
                                                           .withConnectionTimeoutMs(5000)
                                                           .withStatusCode(&statusCode));

            if (instream != nullptr && statusCode == 200) {
                startThread(3);
            } else {
                finish(Result::fail("Failed to start download"));
                return;
            }
        };

        ~DownloadTask()
        {
            stopThread(-1);
        }

        void run() override
        {
            MemoryBlock dekData;

            int64 totalBytes = instream->getTotalLength();
            int64 bytesDownloaded = 0;

            MemoryOutputStream mo(dekData, true);

            while (true) {
                if (threadShouldExit()) {
                    finish(Result::fail("Download cancelled"));
                    return;
                }

                auto written = mo.writeFromInputStream(*instream, 8192);

                if (written == 0)
                    break;

                bytesDownloaded += written;

                float progress = static_cast<long double>(bytesDownloaded) / static_cast<long double>(totalBytes);

                MessageManager::callAsync([this, progress]() mutable {
                    onProgress(progress);
                });
            }

            MemoryInputStream input(dekData, false);
            ZipFile zip(input);

            if (zip.getNumEntries() == 0) {
                finish(Result::fail("The downloaded file was not a valid Deken package"));
                return;
            }

            auto extractedPath = filesystem.getChildFile(packageInfo.name).getFullPathName();
            auto result = zip.uncompressTo(filesystem);

            if (!result.wasOk()) {
                finish(result);
                return;
            }

            // Tell deken about the newly installed package
            manager.addPackageToRegister(packageInfo, extractedPath);

            finish(Result::ok());
        }

        void finish(Result result)
        {
            MessageManager::callAsync(
                [this, result]() mutable {
                    // Make sure lambda still exists after deletion
                    auto finishCopy = onFinish;
                    waitForThreadToExit(-1);

                    // Self-destruct
                    manager.downloads.removeObject(this);

                    finishCopy(result);
                });
        }

        std::function<void(float)> onProgress;
        std::function<void(Result)> onFinish;
    };

    PackageManager()
        : Thread("Deken thread")
    {
        if (!filesystem.exists()) {
            filesystem.createDirectory();
        }

        if (pkgInfo.existsAsFile()) {
            auto newTree = ValueTree::fromXml(pkgInfo.loadFileAsString());
            if (newTree.isValid() && newTree.getType() == Identifier("pkg_info")) {
                packageState = newTree;
            }
        }

        if (cacheFile.existsAsFile()) {
            auto newTree = ValueTree::fromXml(cacheFile.loadFileAsString());
            if (newTree.isValid() && newTree.getType() == Identifier("cache")) {
                cacheState = newTree;
            }
        }

        packageState.addListener(this);
        cacheState.addListener(this);

        sendActionMessage("");
        startThread(3);
    }

    ~PackageManager()
    {
        if (webstream)
            webstream->cancel();
        downloads.clear();
        stopThread(-1);
        clearSingletonInstance();
    }

    void update()
    {
        sendActionMessage("");
        startThread(3);
    }

    void run() override
    {
        // Continue on pipe errors
#ifndef _MSC_VER
        signal(SIGPIPE, SIG_IGN);
#endif
        allPackages = getAvailablePackages();
        sendActionMessage("");
    }

    StringArray getObjectInfo(String const& objectUrl)
    {

        StringArray result;

        webstream = std::make_unique<WebInputStream>(URL("https://deken.puredata.info/info.json?url=" + objectUrl), false);
        webstream->connect(nullptr);

        if (webstream->isError()) {
            sendActionMessage("Failed to connect to Deken server");
            return {};
        }

        // Read json result
        auto json = webstream->readString();

        if (json.isEmpty()) {
            sendActionMessage("Invalid response from Deken server");
            return {};
        }

        try {
            // Parse outer JSON layer
            auto parsedJson = json::parse(json.toStdString());

            // Read json
            auto objects = (*((*(parsedJson["result"]["libraries"].begin())).begin())).at(0)["objects"];

            for (auto obj : objects) {
                result.add(obj["name"]);
            }
        } catch (json::parse_error& e) {
            sendActionMessage("Invalid response from Deken server");
            return {};
        }

        return result;
    }

    PackageList readFromCache()
    {
        PackageList result;

        auto state = cacheState.getChildWithName("State");
        for (auto package : state) {
            auto name = package.getProperty("Name").toString();
            auto author = package.getProperty("Author").toString();
            auto timestamp = package.getProperty("Timestamp").toString();
            auto url = package.getProperty("URL").toString();
            auto description = package.getProperty("Description").toString();
            auto version = package.getProperty("Version").toString();
            StringArray objects;

            for (auto object : package.getChildWithName("Objects")) {
                objects.add(object.getProperty("Name").toString());
            }

            result.add(PackageInfo(name, author, timestamp, url, description, version, objects));
        }

        return result;
    }

    PackageList getAvailablePackages()
    {
        PackageList packages;

        webstream = std::make_unique<WebInputStream>(URL("https://deken.puredata.info/search.json"), false);
        webstream->connect(nullptr);

        if (webstream->isError()) {
            sendActionMessage("Failed to connect to Deken server");
            return {};
        }

        // Read json result
        auto json = webstream->readString();

        if (json.isEmpty()) {
            sendActionMessage("Invalid response from Deken server");
            return {};
        }

        auto checksum = MD5::encode(json);

        // Caching: don't update objects if the index file is the same!
        // Getting all the objects from the packages takes a long time
        auto cachedContent = cacheState.getProperty("Index").toString();
        if (cachedContent.isNotEmpty() && cachedContent == checksum) {
            return readFromCache();
        }

        cacheState.setProperty("Index", checksum, nullptr);

        if (threadShouldExit())
            return {};

        // In case the JSON is invalid
        try {
            // JUCE json parsing unfortunately fails to parse deken's json...
            auto parsedJson = json::parse(json.toStdString());

            // Read json
            auto object = parsedJson["result"]["libraries"];

            // Valid result, go through the options
            for (auto const versions : object) {
                PackageList results;

                if (threadShouldExit())
                    return {};

                // Loop through the different versions
                for (auto v : versions) {
                    // Loop through architectures
                    for (auto arch : v) {
                        auto archs = arch["archs"];
                        // Look for matching platform
                        String platform = archs[0].is_null() ? "" : archs[0];

                        if (checkArchitecture(platform)) {
                            // Extract info
                            String name = arch["name"];
                            String author = arch["author"];
                            String timestamp = arch["timestamp"];

                            String description = arch["description"];
                            String url = arch["url"];
                            String version = arch["version"];

                            StringArray objects = getObjectInfo(url);

                            // Add valid option
                            results.add({ name, author, timestamp, url, description, version, objects });
                        }
                    }
                }

                if (!results.isEmpty()) {
                    // Sort by alphabetically by timestamp to get latest version
                    // The timestamp format is yyyy:mm::dd hh::mm::ss so this should work
                    std::sort(results.begin(), results.end(), [](auto const& result1, auto const& result2) { return result1.timestamp.compare(result2.timestamp) > 0; });

                    auto info = results.getReference(0);
                    packages.addIfNotAlreadyThere(info);
                }
            }
        } catch (json::parse_error& e) {
            sendActionMessage("Invalid response from Deken server");
        }

        ValueTree packageCache = ValueTree("State");

        for (auto& package : packages) {
            ValueTree pkgEntry = ValueTree("Package");
            pkgEntry.setProperty("Name", package.name, nullptr);
            pkgEntry.setProperty("ID", package.packageId, nullptr);
            pkgEntry.setProperty("Author", package.author, nullptr);
            pkgEntry.setProperty("Timestamp", package.timestamp, nullptr);
            pkgEntry.setProperty("Description", package.description, nullptr);
            pkgEntry.setProperty("Version", package.version, nullptr);
            pkgEntry.setProperty("URL", package.url, nullptr);

            ValueTree objects("Objects");
            for (auto& object : package.objects) {
                auto objectTree = ValueTree("Object");
                objectTree.setProperty("Name", object, nullptr);
                objects.appendChild(objectTree, nullptr);
            }

            pkgEntry.appendChild(objects, nullptr);
            packageCache.appendChild(pkgEntry, nullptr);
        }

        cacheState.removeChild(cacheState.indexOf(cacheState.getChildWithName("State")), nullptr);
        cacheState.appendChild(packageCache, nullptr);

        return packages;
    }

    static bool checkArchitecture(String platform)
    {
        // Check OS
        if (platform.upToFirstOccurrenceOf("-", false, false) != os)
            return false;
        platform = platform.fromFirstOccurrenceOf("-", false, false);

        // Check floatsize
        if (platform.fromLastOccurrenceOf("-", false, false) != floatsize)
            return false;
        platform = platform.upToLastOccurrenceOf("-", false, false);

        if (machine.contains(platform))
            return true;

        return false;
    }

    // When a property in our pkginfo changes, save it immediately
    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, Identifier const& property) override
    {
        if (treeWhosePropertyHasChanged == packageState) {
            pkgInfo.replaceWithText(packageState.toXmlString());
        } else if (treeWhosePropertyHasChanged == cacheState) {
            cacheFile.replaceWithText(cacheState.toXmlString());
        }
    }

    void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded) override
    {
        if (parentTree == packageState) {
            pkgInfo.replaceWithText(packageState.toXmlString());
        } else if (parentTree == cacheState) {
            cacheFile.replaceWithText(cacheState.toXmlString());
        }
    }

    void valueTreeChildRemoved(ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override
    {
        if (parentTree == packageState) {
            pkgInfo.replaceWithText(packageState.toXmlString());
        } else if (parentTree == cacheState) {
            cacheFile.replaceWithText(cacheState.toXmlString());
        }
    }

    void uninstall(PackageInfo& packageInfo)
    {
        auto toRemove = packageState.getChildWithProperty("ID", packageInfo.packageId);
        if (toRemove.isValid()) {
            auto folder = File(toRemove.getProperty("Path").toString());
            folder.deleteRecursively();
            packageState.removeChild(toRemove, nullptr);
        }
    }

    DownloadTask* install(PackageInfo packageInfo)
    {
        // Make sure https is used
        packageInfo.url = packageInfo.url.replaceFirstOccurrenceOf("http://", "https://");
        return downloads.add(new DownloadTask(*this, packageInfo));
    }

    void addPackageToRegister(PackageInfo const& info, String path)
    {
        ValueTree pkgEntry = ValueTree(info.name);
        pkgEntry.setProperty("ID", info.packageId, nullptr);
        pkgEntry.setProperty("Author", info.author, nullptr);
        pkgEntry.setProperty("Timestamp", info.timestamp, nullptr);
        pkgEntry.setProperty("Description", info.description, nullptr);
        pkgEntry.setProperty("Version", info.version, nullptr);
        pkgEntry.setProperty("Path", path, nullptr);
        pkgEntry.setProperty("URL", info.url, nullptr);

        // Prevent duplicate entries
        if (packageState.getChildWithProperty("ID", info.packageId).isValid()) {
            packageState.removeChild(packageState.getChildWithProperty("ID", info.packageId), nullptr);
        }
        packageState.appendChild(pkgEntry, nullptr);
    }

    bool packageExists(PackageInfo const& info)
    {
        return packageState.getChildWithProperty("ID", info.packageId).isValid();
    }

    // Checks if the current package is already being downloaded
    DownloadTask* getDownloadForPackage(PackageInfo& info)
    {
        for (auto* download : downloads) {
            if (download->packageInfo == info) {
                return download;
            }
        }

        return nullptr;
    }

    PackageList allPackages;

    inline static File filesystem = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("PlugData").getChildFile("Library").getChildFile("Deken");

    // Package info file
    File pkgInfo = filesystem.getChildFile(".pkg_info");
    File cacheFile = filesystem.getChildFile(".cache");

    // Package state tree, keeps track of which packages are installed and saves it to pkgInfo
    ValueTree packageState = ValueTree("pkg_info");
    ValueTree cacheState = ValueTree("cache");

    // Thread for unzipping and installing packages
    OwnedArray<DownloadTask> downloads;

    std::unique_ptr<WebInputStream> webstream;

    static inline const String floatsize = String(PD_FLOATSIZE);
    static inline const String os =
#if JUCE_LINUX
        "Linux"
#elif JUCE_MAC
        "Darwin"
#elif JUCE_WINDOWS
        "Windows"
    // PlugData has no official BSD support and testing, but for completeness:
#elif defined __FreeBSD__
        "FreeBSD"
#elif defined __NetBSD__
        "NetBSD"
#elif defined __OpenBSD__
        "OpenBSD"
#else
#    if defined(__GNUC__)
#        warning unknown OS
#    endif
        0
#endif
        ;

    static inline const StringArray machine =
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
        { "amd64", "x86_64" }
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_M_IX86)
        { "i386", "i686", "i586" }
#elif defined(__ppc__)
        { "ppc", "PowerPC" }
#elif defined(__aarch64__)
        { "arm64" }
#elif __ARM_ARCH == 6 || defined(__ARM_ARCH_6__)
        { "armv6", "armv6l", "arm" }
#elif __ARM_ARCH == 7 || defined(__ARM_ARCH_7__)
        { "armv7l", "armv7", "armv6l", "armv6", "arm" }
#else
#    if defined(__GNUC__)
#        warning unknown architecture
#    endif
        {}
#endif
    ;

    // Create a single package manager that exists even when the dialog is not open
    // This allows more efficient pre-fetching of packages, and also makes it easy to
    // continue downloading when the dialog closes
    // Inherits from deletedAtShutdown to handle cleaning up
    JUCE_DECLARE_SINGLETON(PackageManager, false)
};

JUCE_IMPLEMENT_SINGLETON(PackageManager)

class Deken : public Component
    , public ListBoxModel
    , public ScrollBar::Listener
    , public ActionListener {

public:
    Deken()
    {
        setInterceptsMouseClicks(false, true);

        listBox.setModel(this);
        listBox.setRowHeight(32);
        listBox.setOutlineThickness(0);
        listBox.deselectAllRows();
        listBox.getViewport()->setScrollBarsShown(true, false, false, false);
        listBox.addMouseListener(this, true);
        listBox.setColour(ListBox::backgroundColourId, Colours::transparentBlack);
        listBox.getViewport()->getVerticalScrollBar().addListener(this);

        input.setJustification(Justification::centredLeft);
        input.setBorder({ 1, 23, 3, 1 });
        input.setName("sidebar::searcheditor");
        input.onTextChange = [this]() {
            filterResults();
        };

        clearButton.setName("statusbar:clearsearch");
        clearButton.setAlwaysOnTop(true);
        clearButton.onClick = [this]() {
            input.clear();
            input.giveAwayKeyboardFocus();
            input.repaint();
            filterResults();
        };

        updateSpinner.setAlwaysOnTop(true);

        addAndMakeVisible(clearButton);
        addAndMakeVisible(listBox);
        addAndMakeVisible(input);
        addAndMakeVisible(updateSpinner);

        refreshButton.setTooltip("Refresh packages");
        refreshButton.setName("statusbar:refresh");
        addAndMakeVisible(refreshButton);
        refreshButton.setConnectedEdges(12);
        refreshButton.onClick = [this]() {
            packageManager->startThread(3);
            packageManager->sendActionMessage("");
        };

        if (packageManager->isThreadRunning()) {
            input.setEnabled(false);
            refreshButton.setEnabled(false);
            clearButton.setEnabled(false);
            input.setText("Updating Packages...");
            updateSpinner.startSpinning();
        } else {
            updateSpinner.setVisible(false);
        }

        packageManager->addActionListener(this);
        filterResults();
    }

    ~Deken()
    {
        packageManager->removeActionListener(this);
    }

    // Package update starts
    void actionListenerCallback(String const& message) override
    {

        auto* thread = dynamic_cast<Thread*>(packageManager);
        bool running = thread->isThreadRunning();

        // Handle errors
        if (message.isNotEmpty()) {
            showError(message);
            input.setEnabled(false);
            updateSpinner.stopSpinning();
            return;
        }

        if (running) {

            input.setText("Updating packages...");
            input.setEnabled(false);
            refreshButton.setEnabled(false);
            clearButton.setEnabled(false);
            updateSpinner.startSpinning();
        } else {
            // Clear text if it was previously disabled
            // If it wasn't, this is just an update call from the package manager
            if (!input.isEnabled()) {
                input.setText("");
            }

            refreshButton.setEnabled(true);
            clearButton.setEnabled(true);
            input.setEnabled(true);
            updateSpinner.stopSpinning();
        }
    }

    void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        repaint();
    }

    void paint(Graphics& g) override
    {
        PlugDataLook::paintStripes(g, 32, listBox.getHeight() + 24, *this, -1, listBox.getViewport()->getViewPositionY() + 4);

        if (errorMessage.isNotEmpty()) {
            g.setColour(Colours::red);
            g.drawText(errorMessage, getLocalBounds().removeFromBottom(28).withTrimmedLeft(8).translated(0, 2), Justification::centredLeft);
        }
    }

    void paintOverChildren(Graphics& g) override
    {
        g.setFont(getLookAndFeel().getTextButtonFont(clearButton, 30));
        g.setColour(findColour(PlugDataColour::textColourId));

        g.drawText(Icons::Search, 0, 0, 30, 30, Justification::centred);

        if (input.getText().isEmpty()) {
            g.setColour(findColour(PlugDataColour::toolbarOutlineColourId));
            g.setFont(Font());
            g.drawText("Type to search for objects or libraries", 32, 0, 350, 30, Justification::centredLeft);
        }

        g.setColour(findColour(PlugDataColour::toolbarOutlineColourId));
        g.drawLine(0, 28, getWidth(), 28);
    }

    int getNumRows() override
    {
        return searchResult.size() + packageManager->downloads.size();
    }

    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
    }

    Component* refreshComponentForRow(int rowNumber, bool isRowSelected, Component* existingComponentToUpdate) override
    {
        delete existingComponentToUpdate;

        if (isPositiveAndBelow(rowNumber, packageManager->downloads.size())) {
            return new DekenRowComponent(*this, packageManager->downloads[rowNumber]->packageInfo);
        } else if (isPositiveAndBelow(rowNumber - packageManager->downloads.size(), searchResult.size())) {
            return new DekenRowComponent(*this, searchResult.getReference(rowNumber - packageManager->downloads.size()));
        }

        return nullptr;
    }

    void filterResults()
    {
        String query = input.getText();

        PackageList newResult;

        searchResult.clear();

        // Show installed packages when query is empty
        if (query.isEmpty()) {
            // make sure installed packages are sorted alphabetically
            PackageSorter::sort(packageManager->packageState);

            for (auto child : packageManager->packageState) {
                auto name = child.getType().toString();
                auto description = child.getProperty("Description").toString();
                auto timestamp = child.getProperty("Timestamp").toString();
                auto url = child.getProperty("URL").toString();
                auto version = child.getProperty("Version").toString();
                auto author = child.getProperty("Author").toString();
                auto objects = StringArray();

                auto info = PackageInfo(name, author, timestamp, url, description, version, objects);

                if (!packageManager->getDownloadForPackage(info)) {
                    newResult.addIfNotAlreadyThere(info);
                }
            }

            searchResult = newResult;
            listBox.updateContent();
            updateSpinner.stopSpinning();

            return;
        }

        auto& allPackages = packageManager->allPackages;

        // First check for name match
        for (auto const& result : allPackages) {
            if (result.name.contains(query)) {
                newResult.addIfNotAlreadyThere(result);
            }
        }

        // Then check for description match
        for (auto const& result : allPackages) {
            if (result.description.contains(query)) {
                newResult.addIfNotAlreadyThere(result);
            }
        }

        // Then check for object match
        for (auto const& result : allPackages) {
            if (result.objects.contains(query)) {
                newResult.addIfNotAlreadyThere(result);
            }
        }

        // Then check for author match
        for (auto const& result : allPackages) {
            if (result.author.contains(query)) {
                newResult.addIfNotAlreadyThere(result);
            }
        }

        // Then check for object close match
        for (auto const& result : allPackages) {
            for (auto const& obj : result.objects) {
                if (obj.contains(query)) {
                    newResult.addIfNotAlreadyThere(result);
                }
            }
        }

        // Downloads are already always visible, so filter them out here
        newResult.removeIf([this](PackageInfo const& package) {
            for (const auto* download : packageManager->downloads) {
                if (download->packageInfo == package) {
                    return true;
                }
            }
            return false;
        });

        searchResult = newResult;
        listBox.updateContent();
    }

    void resized() override
    {
        auto tableBounds = getLocalBounds().withTrimmedBottom(30);
        auto inputBounds = tableBounds.removeFromTop(28);

        int const statusbarHeight = 32;
        int const statusbarY = getHeight() - statusbarHeight;
        auto statusbarBounds = Rectangle<int>(2, statusbarY + 6, getWidth() - 6, statusbarHeight);

        input.setBounds(inputBounds);

        clearButton.setBounds(inputBounds.removeFromRight(30));
        updateSpinner.setBounds(inputBounds.removeFromRight(30));

        tableBounds.removeFromLeft(Sidebar::dragbarWidth);
        listBox.setBounds(tableBounds);

        refreshButton.setBounds(statusbarBounds.removeFromRight(statusbarHeight));
    }

    // Show error message in statusbar
    void showError(String const& message)
    {
        errorMessage = message;
        repaint();
    }

private:
    // List component to list packages
    ListBox listBox;

    // Last error message
    String errorMessage;

    // Current search result
    PackageList searchResult;

    TextButton refreshButton = TextButton(Icons::Refresh);

    PackageManager* packageManager = PackageManager::getInstance();

    TextEditor input;
    TextButton clearButton = TextButton(Icons::Clear);

    Spinner updateSpinner;

    // Component representing a search result
    // It holds package info about the package it represents
    // and can
    struct DekenRowComponent : public Component {
        Deken& deken;
        PackageInfo packageInfo;

        TextButton installButton = TextButton(Icons::SaveAs);
        TextButton reinstallButton = TextButton(Icons::Refresh);
        TextButton uninstallButton = TextButton(Icons::Clear);

        float installProgress;
        ValueTree packageState;

        DekenRowComponent(Deken& parent, PackageInfo& info)
            : deken(parent)
            , packageInfo(info)
            , packageState(deken.packageManager->packageState)
        {
            addChildComponent(installButton);
            addChildComponent(reinstallButton);
            addChildComponent(uninstallButton);

            installButton.setName("statusbar:install");
            reinstallButton.setName("statusbar:reinstall");
            uninstallButton.setName("statusbar:uninstall");

            uninstallButton.onClick = [this]() {
                setInstalled(false);
                deken.packageManager->uninstall(packageInfo);
                deken.filterResults();
            };

            reinstallButton.onClick = [this]() {
                auto* downloadTask = deken.packageManager->install(packageInfo);
                attachToDownload(downloadTask);
            };

            installButton.onClick = [this]() {
                auto* downloadTask = deken.packageManager->install(packageInfo);
                attachToDownload(downloadTask);
            };

            // Check if package is already installed
            setInstalled(deken.packageManager->packageExists(packageInfo));

            // Check if already in progress
            if (auto* task = deken.packageManager->getDownloadForPackage(packageInfo)) {
                attachToDownload(task);
            }
        }

        void attachToDownload(PackageManager::DownloadTask* task)
        {
            task->onProgress = [_this = SafePointer(this)](float progress) {
                if (!_this)
                    return;
                _this->installProgress = progress;
                _this->repaint();
            };

            task->onFinish = [_this = SafePointer(this), task](Result result) {
                if (!_this)
                    return;

                if (result.wasOk()) {
                    _this->setInstalled(result);
                    _this->deken.filterResults();
                } else {
                    _this->deken.showError(result.getErrorMessage());
                    _this->deken.filterResults();
                }
            };

            installButton.setVisible(false);
            reinstallButton.setVisible(false);
            uninstallButton.setVisible(false);
        }

        // Enables or disables buttons based on package state
        void setInstalled(bool installed)
        {
            installButton.setVisible(!installed);
            reinstallButton.setVisible(installed);
            uninstallButton.setVisible(installed);
            installProgress = 0.0f;

            repaint();
        }

        void paint(Graphics& g) override
        {
            g.setColour(findColour(ComboBox::textColourId));

            g.setFont(Font());
            g.drawFittedText(packageInfo.name, 5, 0, 200, getHeight(), Justification::centredLeft, 1, 0.8f);

            // draw progressbar
            if (deken.packageManager->getDownloadForPackage(packageInfo)) {
                float width = getWidth() - 90.0f;
                float right = jmap(installProgress, 90.f, width);

                Path downloadPath;
                downloadPath.addLineSegment({ 90, 15, right, 15 }, 1.0f);

                Path fullPath;
                fullPath.addLineSegment({ 90, 15, width, 15 }, 1.0f);

                g.setColour(findColour(PlugDataColour::toolbarOutlineColourId));
                g.strokePath(fullPath, PathStrokeType(11.0f, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));

                g.setColour(findColour(PlugDataColour::highlightColourId));
                g.strokePath(downloadPath, PathStrokeType(8.0f, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));
            } else {
                g.drawFittedText(packageInfo.version, 90, 0, 150, getHeight(), Justification::centredLeft, 1, 0.8f);
                g.drawFittedText(packageInfo.author, 250, 0, 200, getHeight(), Justification::centredLeft, 1, 0.8f);
                g.drawFittedText(packageInfo.timestamp, 440, 0, 200, getHeight(), Justification::centredLeft, 1, 0.8f);
            }
        }

        void resized() override
        {
            installButton.setBounds(getWidth() - 40, 1, 26, 30);
            uninstallButton.setBounds(getWidth() - 40, 1, 26, 30);
            reinstallButton.setBounds(getWidth() - 70, 1, 26, 30);
        }
    };
};
