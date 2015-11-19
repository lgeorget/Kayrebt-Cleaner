#ifndef NODE_MARKER_H
#define NODE_MARKER_H

#include <vector>
#include <regex>
#include <utility>
#include <string>

#include "mark.h"

class NodeMarker {
private:
	const std::vector<std::pair<std::regex,Mark>> _matcher{
		{std::regex("mutex.*\\("),		Mark::LOCK},
		{std::regex("spin.*lock\\("),		Mark::LOCK},
		{std::regex("atomic_dec_and.*lock\\("),	Mark::LOCK},
		{std::regex("security_.*\\("),		Mark::LSM_HOOK},
		{std::regex("wake_up_new_task\\("),	Mark::FLOW_STMT},
		{std::regex("splice_to_pipe\\("),	Mark::FLOW_STMT},
		{std::regex("do_splice_to\\("),		Mark::FLOW_STMT},
		{std::regex("vma_merge\\("),		Mark::FLOW_STMT},
		{std::regex("vma_link\\("),		Mark::FLOW_STMT},
		{std::regex("__vfs_read\\("),		Mark::FLOW_STMT},
		{std::regex("file_start_write\\("),	Mark::FLOW_STMT},
		{std::regex("do_iter_readv_writev\\("),	Mark::FLOW_STMT},
		{std::regex("do_sync_readv_writev\\("),	Mark::FLOW_STMT},
		{std::regex("do_loop_readv_writev\\("),	Mark::FLOW_STMT},
		{std::regex("copy_page_to_iter\\("),	Mark::FLOW_STMT},
		{std::regex("copy_page_from_iter\\("),	Mark::FLOW_STMT},
		{std::regex("msg_insert\\("),		Mark::FLOW_STMT},
		{std::regex("msg_handler\\("),		Mark::FLOW_STMT},
		{std::regex("pipelined_send\\("),	Mark::FLOW_STMT},
		{std::regex("sendmsg\\("),		Mark::FLOW_STMT},
		{std::regex("recvmsg\\("),		Mark::FLOW_STMT},
	};
public:
	Mark operator()(const std::string& symbol) const;
};

#endif
