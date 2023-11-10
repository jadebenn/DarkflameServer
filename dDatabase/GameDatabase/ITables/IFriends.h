#ifndef __IFRIENDS__H__
#define __IFRIENDS__H__

#include <cstdint>
#include <optional>
#include <vector>

class IFriends {
public:
	struct BestFriendStatus {
		uint32_t playerAccountId{};
		uint32_t friendAccountId{};
		uint32_t bestFriendStatus{};
	};

	virtual std::vector<FriendData> GetFriendsList(const uint32_t charId) = 0;
	virtual std::optional<IFriends::BestFriendStatus> GetBestFriendStatus(const uint32_t playerCharacterId, const uint32_t friendCharacterId) = 0;
	virtual void SetBestFriendStatus(const uint32_t playerAccountId, const uint32_t friendAccountId, const uint32_t bestFriendStatus) = 0;
	virtual void AddFriend(const uint32_t playerAccountId, const uint32_t friendAccountId) = 0;
	virtual void RemoveFriend(const uint32_t playerAccountId, const uint32_t friendAccountId) = 0;
};

#endif  //!__IFRIENDS__H__
