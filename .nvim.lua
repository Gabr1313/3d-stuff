-- put the following line in your neovim configuration
-- vim.o.exrc = true
vim.keymap.set("n", "<leader>md", "<CMD>wa<CR><CMD>make debug<CR><CMD>6cw<CR>",     { desc = "[M]ake [D]ebug" })
vim.keymap.set("n", "<leader>ma", "<CMD>wa<CR><CMD>make analyzer<CR><CMD>6cw<CR>", { desc = "[M]ake [A]nalyzer" })
vim.keymap.set("n", "<leader>mf", "<CMD>wa<CR><CMD>make fast<CR><CMD>6cw<CR>",   { desc = "[M]ake [F]ast" })
vim.keymap.set("n", "<leader>mr", "<CMD>wa<CR><CMD>make release<CR><CMD>6cw<CR>",   { desc = "[M]ake [R]elease" })

vim.api.nvim_create_autocmd("LspAttach", {
	callback = function(args)
		local client = vim.lsp.get_client_by_id(args.data.client_id)
		if client then
			client.stop()
		end
	end,
})
