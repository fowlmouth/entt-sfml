#pragma once

#include <SFML/Graphics.hpp>

namespace UI
{

template< typename Asset >
struct AssetRecord
{
  using Self = AssetRecord<Asset>;

  std::string path;
  std::unique_ptr< Asset > asset;

  AssetRecord(const std::string& path, std::unique_ptr< Asset >&& asset)
  : path(path), asset(std::move(asset))
  {
  }

  AssetRecord()
  : asset(nullptr)
  {}

  AssetRecord(Self&& other)
  : path(std::move(other.path)), asset(std::move(other.asset))
  {
  }

  AssetRecord& operator= (Self&& other)
  {
    path = std::move(other.path);
    asset = std::move(other.asset);
    return *this;
  }
};

template< typename Asset, typename Derived >
struct AssetCache
{
  using AssetType = Asset;
  Derived& derived() { return *static_cast< Derived* >(this); }

  std::unordered_map< std::string, AssetRecord<Asset> > cache;

  ~AssetCache()
  {
    cache.clear();
  }

  Asset* get(const std::string& path)
  {
    auto asset = get_asset(path);
    if(!asset)
      return nullptr;
    return asset->asset.get();
  }

  AssetRecord<Asset>* get_asset(const std::string& path)
  {
    const auto iter = cache.find(path);
    if(iter != cache.cend())
      return &iter->second;
    std::unique_ptr< Asset > asset = derived().load(path);
    if(!asset)
      return nullptr;

    cache.insert({ path, AssetRecord<Asset>(path, std::move(asset)) });
    return &cache[path];
  }
};

} // ::UI

