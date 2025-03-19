-- put the following line in your neovim configuration
-- vim.o.exrc = true
vim.keymap.set("n", "<leader>md", "<CMD>wa<CR><CMD>make debug<CR><CMD>copen<CR>", { desc = "[M]ake [D]ebug" })
vim.keymap.set("n", "<leader>mr", "<CMD>wa<CR><CMD>make release<CR><CMD>copen<CR>", { desc = "[M]ake [R]elease" })

vim.api.nvim_create_autocmd("LspAttach", {
	callback = function(args)
		local client = vim.lsp.get_client_by_id(args.data.client_id)
		if client then
			client.stop()
		end
	end,
})
